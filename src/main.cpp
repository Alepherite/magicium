#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <limits.h>
#include <glib.h>
#include <gio/gio.h>

struct AppContext {
    WebKitWebView* web_view = nullptr;
    GtkWidget* window = nullptr;
    bool page_loaded = false;
    std::string config_dir;
    std::string config_file;
};

// ... Các hàm base64, make_data_url_from_file, make_output_path giữ nguyên ...

std::string base64_encode(const std::vector<unsigned char>& data) {
    std::string result;
    if (data.empty()) return result;
    gchar* encoded = g_base64_encode(data.data(), data.size());
    if (encoded != nullptr) {
        result = encoded;
        g_free(encoded);
    }
    return result;
}

std::vector<unsigned char> base64_decode(const std::string& in) {
    std::vector<unsigned char> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (c == '=') break;
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string make_data_url_from_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return "";
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();
    std::string mime = "application/octet-stream";
    gchar* guessed = g_content_type_guess(path.c_str(), nullptr, 0, nullptr);
    if (guessed != nullptr) {
        mime = guessed;
        g_free(guessed);
    }
    if (mime == "application/octet-stream") {
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), [](unsigned char c) { return std::tolower(c); });
        if (lower_path.find(".png") != std::string::npos) mime = "image/png";
        else if (lower_path.find(".jpg") != std::string::npos || lower_path.find(".jpeg") != std::string::npos) mime = "image/jpeg";
        else if (lower_path.find(".webp") != std::string::npos) mime = "image/webp";
        else if (lower_path.find(".bmp") != std::string::npos) mime = "image/bmp";
        else if (lower_path.find(".gif") != std::string::npos) mime = "image/gif";
        else if (lower_path.find(".tif") != std::string::npos || lower_path.find(".tiff") != std::string::npos) mime = "image/tiff";
    }
    std::string encoded = base64_encode(bytes);
    return "data:" + mime + ";base64," + encoded;
}

std::string make_output_path(const std::string& source_path, const std::string& format) {
    std::string directory = ".";
    std::string base_name = "converted";
    if (!source_path.empty()) {
        gchar* dir = g_path_get_dirname(source_path.c_str());
        if (dir != nullptr) {
            directory = dir;
            g_free(dir);
        }
        gchar* basename = g_path_get_basename(source_path.c_str());
        if (basename != nullptr) {
            std::string raw_basename = basename;
            g_free(basename);
            size_t dot_pos = raw_basename.find_last_of('.');
            if (dot_pos != std::string::npos) {
                base_name = raw_basename.substr(0, dot_pos);
            } else {
                base_name = raw_basename;
            }
        }
    }
    std::string safe_format = format;
    std::transform(safe_format.begin(), safe_format.end(), safe_format.begin(), [](unsigned char c) { return std::tolower(c); });
    std::string filename = directory + "/" + base_name + "_converted." + safe_format;
    if (g_file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
        int suffix = 1;
        while (true) {
            std::string candidate = directory + "/" + base_name + "_converted_" + std::to_string(suffix) + "." + safe_format;
            if (!g_file_test(candidate.c_str(), G_FILE_TEST_EXISTS)) {
                filename = candidate;
                break;
            }
            ++suffix;
        }
    }
    return filename;
}

void send_notification(const std::string& output_path) {
    gchar* quoted_title = g_shell_quote("Image converted");
    gchar* quoted_body = g_shell_quote((std::string("Saved to ") + output_path).c_str());
    gchar* command = g_strdup_printf("notify-send -u low %s %s", quoted_title, quoted_body);
    g_spawn_command_line_async(command, nullptr);
    g_free(quoted_title);
    g_free(quoted_body);
    g_free(command);
}

void run_javascript(WebKitWebView* web_view, const std::string& script) {
    webkit_web_view_evaluate_javascript(web_view, script.c_str(), static_cast<gssize>(script.size()), nullptr, nullptr, nullptr, nullptr, nullptr);
}

std::string save_image_from_payload(const std::string& req) {
    size_t first = req.find(',');
    size_t second = req.find(',', first + 1);
    if (first == std::string::npos || second == std::string::npos) {
        return "{\"status\": \"error\"}";
    }
    std::string format = req.substr(0, first);
    std::string base64_data = req.substr(first + 1, second - first - 1);
    std::string source_path = req.substr(second + 1);
    format.erase(std::remove(format.begin(), format.end(), '"'), format.end());
    format.erase(std::remove(format.begin(), format.end(), '['), format.end());
    format.erase(std::remove(format.begin(), format.end(), ']'), format.end());
    std::vector<unsigned char> data = base64_decode(base64_data);
    gchar* unescaped_source_path = g_uri_unescape_string(source_path.c_str(), nullptr);
    if (unescaped_source_path != nullptr) {
        source_path = unescaped_source_path;
        g_free(unescaped_source_path);
    }
    std::string output_path = make_output_path(source_path, format);
    std::ofstream fout(output_path, std::ios::binary);
    if (fout) {
        fout.write(reinterpret_cast<const char*>(data.data()), data.size());
        fout.close();
        send_notification(output_path);
        return "{\"status\": \"success\", \"file\": \"" + output_path + "\"}";
    }
    return "{\"status\": \"error\"}";
}

static void on_save_script_message(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer user_data) {
    (void)manager;
    AppContext* app_context = static_cast<AppContext*>(user_data);
    if (!app_context || !app_context->page_loaded) return;
    JSCValue* value = webkit_javascript_result_get_js_value(result);
    gchar* message = nullptr;
    if (value != nullptr) {
        message = jsc_value_to_string(value);
    }
    std::string payload = message ? message : "";
    g_free(message);
    std::string response = save_image_from_payload(payload);
    gchar* escaped_response = g_strescape(response.c_str(), nullptr);
    gchar* script = g_strdup_printf("window.__handleSaveResponse('%s');", escaped_response);
    run_javascript(app_context->web_view, script);
    g_free(escaped_response);
    g_free(script);
}

static void open_image_file_chooser(AppContext* app_context) {
    if (!app_context || !app_context->window) return;
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Choose Image",
        GTK_WINDOW(app_context->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        nullptr);
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image files");
    gtk_file_filter_add_mime_type(filter, "image/*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename != nullptr) {
            std::string source_path = filename;
            g_free(filename);
            std::string data_url = make_data_url_from_file(source_path);
            if (!data_url.empty()) {
                gchar* escaped_data_url = g_strescape(data_url.c_str(), nullptr);
                gchar* escaped_path = g_strescape(source_path.c_str(), nullptr);
                gchar* script = g_strdup_printf("window.setImageFromDataUrl('%s', '%s');", escaped_data_url, escaped_path);
                run_javascript(app_context->web_view, script);
                g_free(escaped_data_url);
                g_free(escaped_path);
                g_free(script);
            }
        }
    }
    gtk_widget_destroy(dialog);
}

static void on_select_image_script_message(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer user_data) {
    (void)manager;
    (void)result;
    AppContext* app_context = static_cast<AppContext*>(user_data);
    if (app_context) {
        open_image_file_chooser(app_context);
    }
}

/* ADDED: Hàm xử lý lưu theme từ UI truyền xuống config file */
static void on_save_theme_script_message(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer user_data) {
    (void)manager;
    AppContext* app_context = static_cast<AppContext*>(user_data);
    if (!app_context) return;

    JSCValue* value = webkit_javascript_result_get_js_value(result);
    gchar* theme_str = nullptr;
    if (value != nullptr) {
        theme_str = jsc_value_to_string(value);
    }
    std::string theme = theme_str ? theme_str : "default";
    g_free(theme_str);

    // Ghi đè file config.json mới
    std::ofstream fout(app_context->config_file);
    if (fout) {
        fout << "{\n  \"theme\": \"" << theme << "\"\n}";
        fout.close();
    }
}

/* MODIFIED: Hàm đọc theme từ config và gửi vào UI sau khi load xong */
static void apply_saved_theme(AppContext* app_context) {
    std::string theme = ""; // Mặc định trống để UI tự ăn theo OS nếu chưa có config
    
    std::ifstream fin(app_context->config_file);
    if (fin) {
        std::string line;
        while (std::getline(fin, line)) {
            if (line.find("\"theme\"") != std::string::npos) {
                // Parse thô sơ lấy giá trị trong dấu ngoặc kép cuối cùng
                size_t first_quote = line.find(':', line.find("\"theme\""));
                if (first_quote != std::string::npos) {
                    size_t start = line.find('\"', first_quote);
                    size_t end = line.find('\"', start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        theme = line.substr(start + 1, end - start - 1);
                    }
                }
                break;
            }
        }
        fin.close();
    }

    // Gửi theme sang cho Javascript xử lý
    gchar* script = g_strdup_printf("if (window.applyAppTheme) { window.applyAppTheme('%s'); }", theme.c_str());
    run_javascript(app_context->web_view, script);
    g_free(script);
}

static void on_load_changed(WebKitWebView* web_view, WebKitLoadEvent load_event, gpointer user_data) {
    AppContext* app_context = static_cast<AppContext*>(user_data);
    if (load_event == WEBKIT_LOAD_FINISHED) {
        app_context->page_loaded = true;
        apply_saved_theme(app_context);
    }
    (void)web_view;
}

// ... Các phần còn lại của main() giữ nguyên không thay đổi ...
static gboolean on_drag_motion(GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time, gpointer user_data) {
    (void)x; (void)y; (void)user_data;
    gdk_drag_status(context, GDK_ACTION_COPY, time);
    return TRUE;
}

static gboolean on_drag_drop(GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time, gpointer user_data) {
    (void)x; (void)y; (void)user_data;
    gtk_drag_get_data(widget, context, gdk_atom_intern_static_string("text/uri-list"), time);
    return TRUE;
}

static void on_drag_data_received(GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* data, guint info, guint time, gpointer user_data) {
    (void)widget; (void)x; (void)y; (void)info;
    AppContext* app_context = static_cast<AppContext*>(user_data);
    if (!app_context || !app_context->page_loaded) {
        gtk_drag_finish(context, FALSE, FALSE, time);
        return;
    }
    gchar** uris = gtk_selection_data_get_uris(data);
    if (uris != nullptr) {
        for (gchar** uri = uris; *uri != nullptr; ++uri) {
            gchar* path = g_filename_from_uri(*uri, nullptr, nullptr);
            if (path != nullptr) {
                if (g_file_test(path, G_FILE_TEST_IS_REGULAR)) {
                    std::string source_path = path;
                    std::string data_url = make_data_url_from_file(source_path);
                    if (!data_url.empty()) {
                        gchar* escaped_data_url = g_strescape(data_url.c_str(), nullptr);
                        gchar* escaped_path = g_strescape(source_path.c_str(), nullptr);
                        gchar* script = g_strdup_printf("window.setImageFromDataUrl('%s', '%s');", escaped_data_url, escaped_path);
                        run_javascript(app_context->web_view, script);
                        g_free(escaped_data_url);
                        g_free(escaped_path);
                        g_free(script);
                    }
                }
                g_free(path);
                break;
            }
        }
        g_strfreev(uris);
    }
    gtk_drag_finish(context, TRUE, FALSE, time);
}

int main() {
    gtk_init(nullptr, nullptr);

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) return 1;
    exe_path[len] = '\0';

    gchar* dir_path = g_path_get_dirname(exe_path);
    std::string app_dir = dir_path;
    g_free(dir_path);

    std::string url;
    if (g_file_test((app_dir + "/ui/index.html").c_str(), G_FILE_TEST_EXISTS)) {
        url = "file://" + app_dir + "/ui/index.html";
    } else {
        url = "file://" + app_dir + "/../ui/index.html";
    }

    /* ADDED: Khởi tạo đường dẫn config ~/.config/magicium/config.json */
    AppContext app_context;
    app_context.config_dir = std::string(g_get_user_config_dir()) + "/magicium";
    app_context.config_file = app_context.config_dir + "/config.json";

    // Tạo thư mục config nếu chưa tồn tại
    g_mkdir_with_parents(app_context.config_dir.c_str(), 0755);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Converter");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    app_context.window = window;

    WebKitWebView* web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    app_context.web_view = web_view;

    WebKitUserContentManager* user_content_manager = webkit_web_view_get_user_content_manager(web_view);
    webkit_user_content_manager_register_script_message_handler(user_content_manager, "save_converted_image");
    webkit_user_content_manager_register_script_message_handler(user_content_manager, "select_image");
    /* ADDED: Handler mới nhận lệnh lưu theme từ UI */
    webkit_user_content_manager_register_script_message_handler(user_content_manager, "update_config_theme");

    g_signal_connect(user_content_manager, "script-message-received::save_converted_image", G_CALLBACK(on_save_script_message), &app_context);
    g_signal_connect(user_content_manager, "script-message-received::select_image", G_CALLBACK(on_select_image_script_message), &app_context);
    g_signal_connect(user_content_manager, "script-message-received::update_config_theme", G_CALLBACK(on_save_theme_script_message), &app_context);

    g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), &app_context);

    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

    gtk_drag_dest_set(GTK_WIDGET(web_view), GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_COPY);
    gtk_drag_dest_add_uri_targets(GTK_WIDGET(web_view));
    g_signal_connect(GTK_WIDGET(web_view), "drag-motion", G_CALLBACK(on_drag_motion), &app_context);
    g_signal_connect(GTK_WIDGET(web_view), "drag-drop", G_CALLBACK(on_drag_drop), &app_context);
    g_signal_connect(GTK_WIDGET(web_view), "drag-data-received", G_CALLBACK(on_drag_data_received), &app_context);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    webkit_web_view_load_uri(web_view, url.c_str());
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}