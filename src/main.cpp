#include "webview.h"
#include <iostream>
#include <unistd.h>
#include <limits.h>

int main() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return 1;
    }

    std::string url = std::string("file://") + cwd + "/ui/index.html";

    webview::webview w(false, nullptr);
    w.set_title("Minimal Webview App");
    w.set_size(800, 600, WEBVIEW_HINT_NONE);

    w.bind("go_backend_xu_ly", [&](const std::string &req) -> std::string {
        std::cout << "Backend received: " << req << std::endl;
        return "Acknowledged";
    });

    w.navigate(url);
    w.run();
    
    return 0;
}
