from http.server import HTTPServer, SimpleHTTPRequestHandler

class FakeApacheHandler(SimpleHTTPRequestHandler):
    server_version = "Apache/2.4.49"
    #sys_version = ""

    def end_headers(self):
        self.send_header("Server", self.server_version)
        super().end_headers()

if __name__ == "__main__":
    httpd = HTTPServer(("0.0.0.0", 5000), FakeApacheHandler)
    httpd.serve_forever()
