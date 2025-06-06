#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// Global variables
GtkWidget *song_list;
GtkWidget *song_entry;
GtkWidget *freq_entry;
char *fileloc;

void handle_error(const char *msg) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "Error: %s: %s",
        msg,
        strerror(errno));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void load_songs(void) {
    FILE *file = fopen(fileloc, "r");
    if (file == NULL) {
        if (errno != ENOENT) {
            handle_error("Failed to open file");
        }
        return;
    }

    // Clear existing items
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(song_list)));
    gtk_list_store_clear(store);

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, line, -1);
    }
    fclose(file);
}

void save_songs(void) {
    FILE *file = fopen(fileloc, "w");
    if (file == NULL) {
        handle_error("Failed to open file for writing");
        return;
    }

    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(song_list)));
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

    while (valid) {
        char *song;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &song, -1);
        fprintf(file, "%s\n", song);
        g_free(song);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
    fclose(file);
}

void add_song(GtkWidget *widget, gpointer data) {
    (void)widget;  // Suppress unused parameter warning
    (void)data;    // Suppress unused parameter warning
    const char *song = gtk_entry_get_text(GTK_ENTRY(song_entry));
    if (strlen(song) == 0) return;

    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(song_list)));
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, song, -1);

    gtk_entry_set_text(GTK_ENTRY(song_entry), "");
    save_songs();
}

void remove_song(GtkWidget *widget, gpointer data) {
    (void)widget;  // Suppress unused parameter warning
    (void)data;    // Suppress unused parameter warning
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(song_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkListStore *store = GTK_LIST_STORE(model);
        gtk_list_store_remove(store, &iter);
        save_songs();
    }
}

void modify_frequency(GtkWidget *widget, gpointer data) {
    (void)widget;  // Suppress unused parameter warning
    (void)data;    // Suppress unused parameter warning
    const char *freq = gtk_entry_get_text(GTK_ENTRY(freq_entry));
    if (strlen(freq) == 0) return;

    // Validate that the input is a positive integer
    char *endptr;
    long days = strtol(freq, &endptr, 10);
    if (*endptr != '\0' || days <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Please enter a positive number of days");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(song_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        char *song;
        gtk_tree_model_get(model, &iter, 0, &song, -1);
        
        // Find the last space in the song name (if any)
        char *last_space = strrchr(song, ' ');
        char new_song[512];
        
        if (last_space != NULL) {
            // If there's a space, truncate at that point and add new frequency
            size_t base_len = last_space - song;
            strncpy(new_song, song, base_len);
            new_song[base_len] = '\0';
            snprintf(new_song + base_len, sizeof(new_song) - base_len, " %ld", days);
        } else {
            // If no space found, just append the frequency
            snprintf(new_song, sizeof(new_song), "%s %ld", song, days);
        }
        
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, new_song, -1);
        g_free(song);
        save_songs();
    }
}

void enable_service(GtkWidget *widget, gpointer data) {
    (void)widget;  // Suppress unused parameter warning
    (void)data;    // Suppress unused parameter warning

    // Get the path to the install script
    char script_path[512];
    char* homedir = getenv("HOME");
    if (homedir == NULL) {
        handle_error("Failed to get home directory");
        return;
    }
    snprintf(script_path, sizeof(script_path), "%s/.local/bin/install-pif-notify.sh", homedir);

    // Check if the script exists
    if (access(script_path, F_OK) == -1) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Installation script not found. Please run 'make install' first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Check if the script is executable
    if (access(script_path, X_OK) == -1) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Installation script is not executable. Please run:\n"
            "chmod +x %s", script_path);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Run the installation script with pkexec
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "pkexec %s", script_path);
    if (system(cmd) != 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Failed to enable notification service. Please check system logs.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Show success message
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Notification service enabled successfully!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_about(GtkWidget *widget, gpointer data) {
    (void)widget;  // Suppress unused parameter warning
    (void)data;    // Suppress unused parameter warning
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "PIF Song Manager");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© 2024");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_AGPL_3_0);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
        "A tool for managing piano practice songs and their practice frequencies.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void setup_file(void) {
    char* homedir;
    uid_t uid = getuid();

    struct passwd *info = getpwuid(uid);
    if (info == NULL) {
        handle_error("Failed to get user information");
        exit(1);
    }

    homedir = info->pw_dir;
    if (homedir == NULL) {
        handle_error("Home directory is NULL");
        exit(1);
    }

    fileloc = malloc(strlen(homedir) + 6);
    if (fileloc == NULL) {
        handle_error("Memory allocation failed");
        exit(1);
    }
    sprintf(fileloc, "%s/.pif", homedir);

    FILE *file = fopen(fileloc, "r");
    if (file == NULL) {
        if (errno != ENOENT) {
            handle_error("Failed to open file");
            exit(1);
        }

        file = fopen(fileloc, "w");
        if (file == NULL) {
            handle_error("Failed to create file");
            exit(1);
        }
        fclose(file);

        if (chmod(fileloc, 0600) == -1) {
            handle_error("Failed to set file permissions");
            exit(1);
        }
    } else {
        fclose(file);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;  // Suppress unused parameter warning
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *add_button;
    GtkWidget *remove_button;
    GtkWidget *freq_button;
    GtkWidget *scrolled_window;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget *menubar;
    GtkWidget *file_menu;
    GtkWidget *help_menu;
    GtkWidget *file_item;
    GtkWidget *help_item;
    GtkWidget *quit_item;
    GtkWidget *about_item;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PIF Song Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create menu bar
    menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // File menu
    file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    file_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    GtkWidget *service_item = gtk_menu_item_new_with_label("Enable Notification Service");
    g_signal_connect(service_item, "activate", G_CALLBACK(enable_service), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), service_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());

    quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect_swapped(quit_item, "activate", G_CALLBACK(gtk_widget_destroy), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

    // Help menu
    help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);
    help_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);

    about_item = gtk_menu_item_new_with_label("About");
    g_signal_connect(about_item, "activate", G_CALLBACK(show_about), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    // Song entry and add button
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    song_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(song_entry), "Enter song name");
    gtk_box_pack_start(GTK_BOX(hbox), song_entry, TRUE, TRUE, 0);

    add_button = gtk_button_new_with_label("Add");
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_song), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);

    // Frequency entry and button
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Add a label to explain the frequency
    GtkWidget *freq_label = gtk_label_new("Practice frequency (days between practices):");
    gtk_box_pack_start(GTK_BOX(hbox), freq_label, FALSE, FALSE, 0);

    freq_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(freq_entry), "Enter number of days");
    gtk_box_pack_start(GTK_BOX(hbox), freq_entry, TRUE, TRUE, 0);

    freq_button = gtk_button_new_with_label("Set Practice Frequency");
    g_signal_connect(freq_button, "clicked", G_CALLBACK(modify_frequency), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), freq_button, FALSE, FALSE, 0);

    // Song list
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    song_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Songs",
                                                    renderer,
                                                    "text", 0,
                                                    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(song_list), column);
    gtk_container_add(GTK_CONTAINER(scrolled_window), song_list);

    // Remove button
    remove_button = gtk_button_new_with_label("Remove Selected");
    g_signal_connect(remove_button, "clicked", G_CALLBACK(remove_song), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), remove_button, FALSE, FALSE, 0);

    // Load existing songs
    load_songs();

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    setup_file();

    app = gtk_application_new("org.pif.gtk", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free(fileloc);
    return status;
} 