/***
 * 
 * primitive wrapper for the OCC API
 *
 * coder@computer.org
 * coder@0xc0d3.org
 *
 *
 */
#ifndef OCCD_H
#define OCCD_H
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <string>
#include <regex>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/range/irange.hpp>

#include "blargh.h"
#include "../space/le_space.h"

//beg OCC includes
#include <config.h>
#include <Standard_Type.hxx>
#include <Standard.hxx>
#include <Standard_Boolean.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <Message_MsgFile.hxx>
#include <TopoDS.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <BRepTools.hxx>
#include <BRepMesh.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <XSControl_WorkSession.hxx>
//end OCC includes

using namespace blargh;
//using namespace space;
static std::string data_root_ = "./data_dump/";

typedef EuklidianSpaceCompressedFuint16< 3 > Space3;
typedef EuklidianSpaceCompressedFuint16< 4 > Space4;

Space4 *s_space4p;


extern "C"
{

}
struct occrest
{
//    std::shared_ptr< XSControl_WorkSession > worksession_p;
//    XSControl_WorkSession worksession;

    //std::shared_ptr< STEPControl_Reader > stepreader_p;
    static void init()
    {
        //STEPControl_Reader reader; //generates a WorkSession
       // XSControll
       //stepreader_p = new STEPC(worksession)
       s_space4p = new Space4;
    }
    struct GET
    {
        ext2type e2t;
        void operator()(const request& req, response& resp)
        {

            boost::regex val_0 
            {"^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"};
            boost::smatch matches;
            if(!boost::regex_match(req.uri, matches, val_0))
            {
                resp = response::default_response(bad_request);
                return;
            }
            std::string path(matches[5]);

            if (path.empty() || path[0] != '/' || path.find("..") 
                    != std::string::npos)
            {
                resp = response::default_response(bad_request);
                return;
            }

            if (path[path.size() - 1] == '/')
            {
                //pseudo-directory foo? capabilities?
                //path += "main.html";
            }

            std::size_t last_slash_pos = path.find_last_of("/");
            std::size_t last_dot_pos = path.find_last_of(".");
            std::string extension;
            if (last_dot_pos != std::string::npos 
                    && last_dot_pos > last_slash_pos)
            {
                extension = path.substr(last_dot_pos + 1);
            }
            std::string full_path = doc_root_ + path;
            std::ifstream is(full_path.c_str(), std::ios::in 
                    | std::ios::binary);
            if (!is)
            {
                resp = response::default_response(status_type::not_found);
                return;
            }

            resp.status = status_type::ok;
            boost::regex stp {"\\w*\\.(stp|step|STP|STEP)"};
            boost::regex iges {"\\w*\\.(igs|IGS|iges|IGES)"};
            std::cerr << "from GET" << std::endl;
            std::cerr << "full_path = " << full_path << std::endl;

            std::stringstream ps;
          //  ps << "[";
            XSControl_WorkSession worksession;
            if(boost::regex_search(full_path, matches, stp))
            {
                std::cerr << "match!" << std::endl;
                STEPControl_Reader stepreader;

                stepreader.ReadFile(full_path.c_str());

                //Handle( TColStd_HSequenceOfTransient ) h
                Standard_Integer NbTrans = stepreader.TransferRoots();
                std::cerr << "NbRootsForTransfer() = " << NbTrans
                    << std::endl;
                TopoDS_Shape resulting_shape = stepreader.OneShape();
                TopoDS_Iterator topo_iter;
                TopExp_Explorer faceExp(resulting_shape, TopAbs_FACE);

                for(; faceExp.More(); faceExp.Next())
                {
                    TopLoc_Location L = faceExp.Current().Location();
                    BRepMesh::Mesh(faceExp.Current(), 1.0);
                    Handle (Poly_Triangulation) facing =
                        BRep_Tool::Triangulation(
                                TopoDS::Face(faceExp.Current()),L);
                    if (!facing.IsNull())
                    {
                        TopExp_Explorer vertexExp(faceExp.Current(),
                                TopAbs_VERTEX);
                        const Poly_Array1OfTriangle & triangles =
                            facing->Triangles();
                        const TColgp_Array1OfPnt & nodes = facing->Nodes();
                        std::cerr << "Number of Triangles: " 
                            << facing->NbTriangles() << std::endl;
                        for ( int i=facing->NbTriangles(); i >= 1; --i )
                        {
                            Poly_Triangle triangle = triangles(i);

                            Standard_Integer node0,node1,node2;
                            triangle.Get(node0, node1, node2);

                            gp_Pnt p[3];
                            p[0] = nodes(node0).Transformed(L);
                            p[1] = nodes(node1).Transformed(L);
                            p[2] = nodes(node2).Transformed(L);

                            Space4::PointT v[3];
                            Space4::PointT d[3]; 
                            for(int n : { 0, 1, 2 })
                            {
                                v[n][0] = 
                                    std::numeric_limits<Space3::ValueType>::
                                    max() +
                                    std::numeric_limits<Space3::ValueType>::
                                    max() * p[n].X();
                                v[n][1] = 
                                    std::numeric_limits<Space3::ValueType>::
                                    max() +
                                    std::numeric_limits<Space3::ValueType>::
                                    max() * p[n].Y();
                                v[n][2] = 
                                    std::numeric_limits<Space3::ValueType>::
                                    max() +
                                    std::numeric_limits<Space3::ValueType>::
                                    max() * p[n].Z();
                            }
                            for(int n : boost::irange(0,2))
                            {
                                d[n] = v[(n + 1) % 3] - v[n];
                            }
                            Space4::PointT q;

                            Space4::PointT::ValueType a, b, c;
                            //ps << "length " << d[0].length();
                            Space4::Metric::KeyType k = 
                                Space4::Metric::morton_encode(v[0]);
                            s_space4p->insert(k);

                        }
                    }
                }
                int size = 5; // s_space4p->size();

                //implement subtree access
               ps << "{"
                  << "{ \"path\": " << size << "}"
                  << "}" << std::endl;
               //ps << "[0,0,0]]" << std::endl;
               char buf[512];
               while (ps.read(buf, sizeof(buf)).gcount() > 0)
               resp.content.append(buf, ps.gcount());
               resp.headers.resize(2);
               resp.headers[0].name = "Content-Length";
               resp.headers[0].value = 
               boost::lexical_cast<std::string>(resp.content.size());
               resp.headers[1].name = "Content-Type";
               resp.headers[1].value = e2t[extension];
               



            }




        }
    };

    struct HEAD
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct POST
    {
        void operator()(const request& req, response& resp)
        {
            std::string filename(data_root_);
            boost::regex val_0 
            {"^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"};
            boost::smatch matches;

            if(!boost::regex_match(req.uri, matches, val_0))
            {
                resp = response::default_response(status_type::bad_request);
                return;
            }
            std::string path(matches[5]);

            if (path.empty() || path[0] != '/' || path.find("..")
                    != std::string::npos)
            {
                resp = response::default_response(status_type::bad_request);
                return;
            }
            boost::regex val_1 
            {"\\[([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*),([-+]?[0-9]*\\.?[0-9]*)\\]"};
            if(!boost::regex_match(req.content, matches, val_1))return;
            resp = response::default_response(status_type::ok);

        }
    };

    struct PUT
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct DELETE
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct TRACE
    {
        void operator()(const request& req, response& resp)
        {
        }
    };

    struct CONNECT
    {
        void operator()(const request& req, response& resp)
        {}
    };

};
#endif
