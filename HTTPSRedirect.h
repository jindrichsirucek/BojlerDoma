#pragma once

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#ifdef HTTPS_REDIRECT_DEBUG
#else
    #define HTTPS_REDIRECT_DEBUG false
#endif

#ifdef DEBUG_OUTPUT
#else
    #define DEBUG_OUTPUT Serial
#endif

class HTTPSRedirect : public WiFiClientSecure {
public:
    HTTPSRedirect(int port = 443) : m_port(port)
    { }

    bool GET(const String &host, const String &url)
    {
        if (!con(host, m_port)) return false;

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.print("requesting URL: ");
        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println(url);

        print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Accept: */*\r\n" +
              "Connection: close\r\n\r\n");

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("request sent");
        while (available() == 0) {
            displayPrint(".");
            if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.print(".");
            delay(300);
        }

        return processHeader();
    }

    bool POST(const String &host, const String &url, const String &data)
    {
        if (!con(host, m_port)) return false;

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.print("requesting URL: ");
        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println(url);

        print(String("POST ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Accept: */*\r\n" +
              "Content-Length: " + data.length() + "\r\n" +
              "Connection: close\r\n\r\n");
        print(data);

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("request sent ");
        while (available() == 0) {
            displayPrint(".");
            if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.print(".");
            delay(300);
        }

        return processHeader();
    }

protected:
    int m_port;

    bool con(const String &host, const int &port)
    {
        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.print("connecting to ");
        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println(host);
        if (!connect(host.c_str(), port)) {
            if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("connection failed");
            return false;
        }

        return true;
    }

    bool processHeader()
    {
        while (available()) {
            String line = readStringUntil('\n');
            // if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("line: " + line);
            if (line.startsWith("HTTP/1.1 302 Moved Temporarily")) {
                return redirect();
            }
            else if (line == "\r") {
                if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("header received");
                return true;
            }
        }

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("header not complete");
        return false;
    }

    bool redirect()
    {
        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("redirecting");

        String locationStr = "Location: ";
        while (connected()) {
            String line = readStringUntil('\n');
            if (line.startsWith(locationStr)) {
                String address = line.substring(locationStr.length());
                String redirHost, redirUrl;
                parseUrl(address, redirHost, redirUrl);

                return GET(redirHost, redirUrl);
            }
            else if (line == "\r") {
                if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("no redirection location found");
                return false;
            }
        }

        if (HTTPS_REDIRECT_DEBUG) DEBUG_OUTPUT.println("header not complete");
        return false;
    }

    void parseUrl(const String &address, String &host, String &url)
    {
        int protocolEnd = address.indexOf("://");

        int hostBegin = protocolEnd;
        if (protocolEnd != 0) {
            hostBegin += 3;
        }

        int urlBegin = address.indexOf('/', hostBegin);
        host = address.substring(hostBegin, urlBegin);
        url = address.substring(urlBegin);
    }
};
