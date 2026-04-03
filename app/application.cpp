#include <iostream>
#include "./server.hpp"

void handleMainRoute(HttpRequest *req, HttpResponse *res)
{
    res->setStatusCode(HttpStatusCode::OK);
    res->addBody("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Landing</title><style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:0;color:#111;background:#f7f9fc}header{background:#0b79d0;color:#fff;padding:2rem;text-align:center}main{padding:2rem;max-width:800px;margin:1rem auto}a.button{display:inline-block;background:#0b79d0;color:#fff;padding:.6rem 1rem;border-radius:6px;text-decoration:none}footer{font-size:.9rem;color:#666;text-align:center;padding:1.5rem}nav a{color:#fff;margin:0 .5rem;text-decoration:none}</style></head><body><header><h1>Welcome to Example</h1><nav><a href=\"/about\">About</a>|<a href=\"/contact\">Contact</a></nav></header><main><h2>Simple, fast starter</h2><p>A lightweight landing page to introduce your project. Clean layout, responsive, and ready to customize.</p><p><a class=\"button\" href=\"/contact\">Get in touch</a></p></main><footer>© 2026 Example. All rights reserved.</footer></body></html>");
    res->addHeader("Content-Type", "text/html");
}

void handleContactRoute(HttpRequest *req, HttpResponse *res)
{
    res->setStatusCode(HttpStatusCode::OK);
    res->addBody("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Contact</title><style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;background:#f4f6f8;color:#111;margin:0}main{max-width:640px;margin:2rem auto;padding:1.5rem;background:#fff;border-radius:8px;box-shadow:0 2px 6px rgba(16,24,40,.06)}h1{color:#0b79d0}form{display:grid;gap:.75rem}label{font-size:.9rem;color:#333}input[type=text],input[type=email],textarea{width:100%;padding:.6rem;border:1px solid #dfe6ef;border-radius:6px;font:inherit}button{background:#0b79d0;color:#fff;border:none;padding:.6rem 1rem;border-radius:6px;cursor:pointer}small{color:#666}a{color:#0b79d0}</style></head><body><main><h1>Contact</h1><p>Send us a message and we’ll get back shortly.</p><form action=\"/contact\" method=\"post\"><label>Name<br><input type=\"text\" name=\"name\" required></label><label>Email<br><input type=\"email\" name=\"email\" required></label><label>Message<br><textarea name=\"message\" rows=\"5\" required></textarea></label><button type=\"submit\">Send</button><p><small>Alternatively, email us at <a href=\"mailto:hello@example.com\">hello@example.com</a></small></p></form></main></body></html>");
    res->addHeader("Content-Type", "text/html");
}

void handleAboutRoute(HttpRequest *req, HttpResponse *res)
{
    res->setStatusCode(HttpStatusCode::OK);
    res->addBody("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>About</title><style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:0;color:#222;background:#fff}main{padding:2rem;max-width:780px;margin:2rem auto}h1{color:#0b79d0}section{margin-bottom:1.25rem}dl{display:grid;grid-template-columns:140px 1fr;gap:.25rem .75rem}dt{font-weight:600;color:#444}dd{margin:0;color:#555}footer{font-size:.85rem;color:#888;text-align:center;padding:1.5rem}nav a{color:#0b79d0;text-decoration:none;margin-right:.5rem}</style></head><body><main><nav><a href=\"/\">Home</a></nav><h1>About This Project</h1><section><p>This project is a minimal example demonstrating three endpoints with small HTML pages. It focuses on clarity and accessibility.</p></section><section><h2>Details</h2><dl><dt>Tech</dt><dd>Plain HTML + minimal CSS</dd><dt>Purpose</dt><dd>Starter template for small sites</dd></dl></section><footer>Made with care — 2026</footer></main></body></html>");
    res->addHeader("Content-Type", "text/html");
}

int main()
{
    Server serv = Server("localhost", "8080");

    serv.addRoute("/", &handleMainRoute);
    serv.addRoute("/contact", &handleContactRoute);
    serv.addRoute("/about", &handleAboutRoute);

    serv.launch();

    return 0;
}