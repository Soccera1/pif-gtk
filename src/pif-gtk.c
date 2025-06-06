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
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(song_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkListStore *store = GTK_LIST_STORE(model);
        gtk_list_store_remove(store, &iter);
        save_songs();
    }
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
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *add_button;
    GtkWidget *remove_button;
    GtkWidget *scrolled_window;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PIF Song Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Song entry and add button
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    song_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), song_entry, TRUE, TRUE, 0);

    add_button = gtk_button_new_with_label("Add");
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_song), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);

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

    app = gtk_application_new("org.pif.gtk", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free(fileloc);
    return status;
} 