#ifndef _WEB_VIZ_H_
#define _WEB_VIZ_H_
// #include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>

#include <boost/beast/core.hpp>
// #include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <jsoncpp/json/json.h> 

#include "Parameter/Parameter.h"
#include "Util/pcl_utils.hpp"
#include <time.h>
namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class WebViz
{
    public :
        WebViz(){cloud.reset(new pcl::PointCloud<PointXYZIRL>);};
        void init(std::string addr, int port, std::string doc_root)
        {
            param.insertParam("address", addr);
            param.insertParam("port", port);
            param.insertParam("doc_root", doc_root);
            return;
        };
        void run()
        {
            acceptor_th = std::thread(acceptor_handler,this);
            acceptor_th.detach();
            return;
        }
        // void setPointCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr inCld)
        // {
        //     // cloud->clear();
        //     // pcl::copyPointCloud(*inCld, *cloud);
        //     cloud = inCld;
        //     return;
        // }
        void setPointCloud(pcl::PointCloud<PointXYZIRL>::Ptr inCld)
        {
            cloud = inCld;
            return;
        }
        // void setPointCloud(pcl::PointCloud<pcl::PointXYZINormal>::Ptr inCld)
        // {
        //     cloud->clear();
        //     for(int i =0 ;  i < inCld->points.size();i++)
        //     {
        //         pcl::PointXYZ p;
        //         p.x = inCld->points[i].x;
        //         p.y = inCld->points[i].y;
        //         p.z = inCld->points[i].z;
        //         cloud->points.push_back(p);
        //     }
        //     return;
        // }
    private:
        pcl::PointCloud<PointXYZIRL>::Ptr cloud;
        std::string convertPointCloudData() 
        {
            Json::Value root;
            Json::StreamWriterBuilder writer;
            
            // Example data - replace with your actual point cloud data
            for(int i = 0 ; i < cloud->points.size();i++)
            {
                Json::Value p;
                p["x"] = cloud->points[i].x;
                p["y"] = cloud->points[i].y;
                p["z"] = cloud->points[i].z;
                p["l"] = cloud->points[i].label;
                root.append(p);
            }

            return Json::writeString(writer, root);
        }
        static void *acceptor_handler(void *_pthis)
        {
            WebViz *pthis = (WebViz *)_pthis;
            pthis->acceptor();
            return NULL;
        }
        static void *session_handler(void *_pthis, tcp::socket &socket, std::shared_ptr<std::string const> const &doc_root)
        {
            WebViz *pthis = (WebViz *)_pthis;
            pthis->handleSession(socket, doc_root);
            return NULL;
        }
        void handleSession(tcp::socket &socket, std::shared_ptr<std::string const> const &doc_root)
        {
            beast::error_code ec;

            // This buffer is required to persist across reads
            beast::flat_buffer buffer;
            std::cout << "session Start!\n";
            while(1)
            {
                // Read a request
                http::request<http::string_body> req;
                http::read(socket, buffer, req, ec);
                if (ec == http::error::end_of_stream)
                    break;
                if (ec)
                    return fail(ec, "read");

                // Handle request
                http::message_generator msg =
                    handleRequest(*doc_root, std::move(req));
                // Determine if we should close the connection
                bool keep_alive = msg.keep_alive();

                // Send the response
                beast::write(socket, std::move(msg), ec);
            }
            std::cout << "session End!\n";
            // Send a TCP shutdown
            socket.shutdown(tcp::socket::shutdown_send, ec);
        }
        template <class Body, class Allocator> 
        http::message_generator handleRequest(beast::string_view doc_root, 
                                    http::request<Body, http::basic_fields<Allocator>> &&req)
        {

            // Make sure we can handle the method
            if (req.method() != http::verb::get &&
                req.method() != http::verb::head)
                return badReqResponse(std::move(req), "Unknown HTTP-method");

            // Request path must be absolute and not contain "..".
            if (req.target().empty() ||
                req.target()[0] != '/' ||
                req.target().find("..") != beast::string_view::npos)
                return badReqResponse(std::move(req), "Illegal request-target");

            // Build the path to the requested file

            std::string path = path_cat(doc_root, req.target());
            if (req.target().back() == '/')
                path.append("index.html");

            if (req.target() == "/pointcloud") 
                return pointcloudResponse(std::move(req));
            // Attempt to open the file
            beast::error_code ec;
            http::file_body::value_type body;
            body.open(path.c_str(), beast::file_mode::scan, ec);

            // Handle the case where the file doesn't exist
            if (ec == beast::errc::no_such_file_or_directory)
                return notFoundResponse(std::move(req), req.target());

            // Handle an unknown error
            if (ec)
                return serverErrorResponse(std::move(req), ec.message());

            // Cache the size since we need it after the move

            // Respond to HEAD request
            if (req.method() == http::verb::head)
                return headResponse(std::move(req), std::move(body), path);
            // Respond to GET request
            return fileResponse(std::move(req), std::move(body), path);

            // return fileResponse(std::move(req), std::move(body), path);
        }
        void acceptor()
        {
            try
            {
                auto const address = net::ip::make_address(param.getParamStr("address").c_str());
                auto const port = static_cast<unsigned short>(param.getParamInt("port"));
                auto const doc_root = std::make_shared<std::string>(param.getParamStr("doc_root"));

                net::io_context ioc{1};

                // The acceptor receives incoming connections
                tcp::acceptor acceptor{ioc, {address, port}};
                while (1)
                {
                    // This will receive the new connection
                    tcp::socket socket{ioc};

                    // Block until we get a connection
                    acceptor.accept(socket);

                    // Launch the session, transferring ownership of the socket
                    std::thread{std::bind(
                                    session_handler, this,
                                    std::move(socket),
                                    doc_root)}
                        .detach();
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
                return;
            }
            return;
        }
        void fail(beast::error_code ec, char const *what)
        {
            std::cerr << what << ": " << ec.message() << "\n";
            return;
        }
        beast::string_view mime_type(beast::string_view path)
        {
            using beast::iequals;
            auto const ext = [&path]
            {
                auto const pos = path.rfind(".");
                if (pos == beast::string_view::npos)
                    return beast::string_view{};
                return path.substr(pos);
            }();
            if (iequals(ext, ".htm"))
                return "text/html";
            if (iequals(ext, ".html"))
                return "text/html";
            if (iequals(ext, ".php"))
                return "text/html";
            if (iequals(ext, ".css"))
                return "text/css";
            if (iequals(ext, ".txt"))
                return "text/plain";
            if (iequals(ext, ".js"))
                return "application/javascript";
            if (iequals(ext, ".json"))
                return "application/json";
            if (iequals(ext, ".xml"))
                return "application/xml";
            if (iequals(ext, ".swf"))
                return "application/x-shockwave-flash";
            if (iequals(ext, ".flv"))
                return "video/x-flv";
            if (iequals(ext, ".png"))
                return "image/png";
            if (iequals(ext, ".jpe"))
                return "image/jpeg";
            if (iequals(ext, ".jpeg"))
                return "image/jpeg";
            if (iequals(ext, ".jpg"))
                return "image/jpeg";
            if (iequals(ext, ".gif"))
                return "image/gif";
            if (iequals(ext, ".bmp"))
                return "image/bmp";
            if (iequals(ext, ".ico"))
                return "image/vnd.microsoft.icon";
            if (iequals(ext, ".tiff"))
                return "image/tiff";
            if (iequals(ext, ".tif"))
                return "image/tiff";
            if (iequals(ext, ".svg"))
                return "image/svg+xml";
            if (iequals(ext, ".svgz"))
                return "image/svg+xml";
            return "application/text";
        }
        std::string path_cat(beast::string_view base, beast::string_view path)
        {
            if (base.empty())
                return std::string(path);
            std::string result(base);
        #ifdef BOOST_MSVC
            char constexpr path_separator = '\\';
            if (result.back() == path_separator)
                result.resize(result.size() - 1);
            result.append(path.data(), path.size());
            for (auto &c : result)
                if (c == '/')
                    c = path_separator;
        #else
            char constexpr path_separator = '/';
            if (result.back() == path_separator)
                result.resize(result.size() - 1);
            result.append(path.data(), path.size());
        #endif
            return result;
        }
        
        template <class Body, class Allocator>
        http::response<http::string_body> badReqResponse(http::request<Body, http::basic_fields<Allocator>> &&req, beast::string_view why)
        {
            http::response<http::string_body> res{http::status::bad_request, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::string(why);
            res.prepare_payload();
            return res;
        }
        template <class Body, class Allocator>
        http::response<http::string_body> notFoundResponse(http::request<Body, http::basic_fields<Allocator>> &&req, beast::string_view target)
        {
            http::response<http::string_body> res{http::status::not_found, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "The resource '" + std::string(target) + "' was not found.";
            res.prepare_payload();
            return res;
        }
        template <class Body, class Allocator>
        http::response<http::string_body> serverErrorResponse(http::request<Body, http::basic_fields<Allocator>> &&req, beast::string_view what)
        {
            http::response<http::string_body> res{http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        }
        template <class Body, class Allocator>
        http::response<http::empty_body> headResponse(http::request<Body, http::basic_fields<Allocator>> &&req, http::file_body::value_type body, std::string path)
        {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(body.size());
            res.keep_alive(req.keep_alive());
            return res;
        }
        template <class Body, class Allocator>
        http::response<http::file_body> fileResponse(http::request<Body, http::basic_fields<Allocator>> &&req, http::file_body::value_type body, std::string path)
        {
            http::response<http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(http::status::ok, req.version())};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(body.size());
            res.keep_alive(req.keep_alive());
            return res;
        }
        template <class Body, class Allocator>
        http::response<http::string_body> pointcloudResponse(http::request<Body, http::basic_fields<Allocator>> &&req)
        {
            std::string data = convertPointCloudData();
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = data;
            res.prepare_payload();
            return res;
        }
    private:
        Parameter param;
        std::thread acceptor_th;
};
#endif