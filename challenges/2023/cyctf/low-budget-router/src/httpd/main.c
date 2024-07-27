#include "httpd.h"

void GETHomePage()
{
    if (getLogin())
        renderHtml("/home/httpd/home.html");
    else
        renderHtml("/home/httpd/index.html");
}

void GETLogin()
{
    if (getLogin())
        renderHtml("/home/httpd/home.html");
    else
        renderHtml("/home/httpd/login.html");
}

void POSTLogin()
{
    unsigned int size = strlen(payload);
    char buffer[80];
    if (size >= 80)
    {
        serverError();
        return;
    }
    memcpy(buffer, payload, payload_size);
    if (!strncmp(buffer, "user=admin&pass=notcomplexjustnotguessable", size))
    {
        setLogin(1);
        redirect("/");
    }
    else
    {
        setLogin(0);
        redirect("/login");
    }
}

void GETLogout()
{
    if (getLogin())
        setLogin(0);
    redirect("/");
}

void GETStatus()
{
    if (getLogin())
    {
        renderHtml("/home/httpd/status.html");
    }
    else
        redirect("/");
}

void GETFlag()
{
    if (getLogin())
    {
        renderHtml("/home/httpd/flag.html");
    }
    else
        redirect("/");
}

void route()
{
    ROUTE_START()

    ROUTE_GET("/")
    {
        GETHomePage();
    }

    ROUTE_GET("/login")
    {
        GETLogin();
    }

    ROUTE_POST("/login")
    {
        POSTLogin();
    }

    ROUTE_GET("/logout")
    {
        GETLogout();
    }

    ROUTE_GET("/status")
    {
        GETStatus();
    }

    ROUTE_GET("/flag")
    {
        GETFlag();
    }

    ROUTE_END()
}

int main(int c, char **v)
{
    setLogin(0);
    serve_forever("10080");
    return 0;
}
