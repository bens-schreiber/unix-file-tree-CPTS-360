#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"

/// @brief Asserts that the condition is true, and if it is not, sets the out_buffer to the error message and returns.
#define out_error(condition, out_buffer, error) \
    if (condition)                              \
    {                                           \
        strcpy(out_buffer, error);              \
        return;                                 \
    }

static linked_list_t *_system_path = NULL;

const linked_list_t *system_path()
{
    return _system_path;
}

void path_init(const file_tree_t *tree)
{
    _system_path = linked_list_init();
    linked_list_insert(_system_path, tree->root);
}

void path_free()
{
    linked_list_free(_system_path);
}

void pwd(out_buffer_t out_buffer)
{
    linked_list_node_t *iter = _system_path->head;
    int index = 0;
    while (iter != NULL)
    {
        file_node_t *node = (file_node_t *)iter->data;
        for (int i = 0; i < strlen(node->name); i++)
        {
            out_buffer[index] = node->name[i];
            index += 1;
        }
        iter = iter->next;
        out_buffer[index] = '/';
        index += 1;
    }
    out_buffer[index] = '\0';
}

void cd(out_buffer_t out_buffer, file_tree_t *tree, const char *path)
{
    out_error(path == NULL || strlen(path) == 0, out_buffer, "err: no directory specified\n");
    linked_list_t *new_system_path = unix_tree_traverse(tree, path);
    out_error(new_system_path == NULL, out_buffer, "err: directory not found\n");

    linked_list_free(_system_path);
    _system_path = new_system_path;
}

void ls(out_buffer_t out_buffer, file_tree_t *tree, const char *path)
{
    if (path == NULL || strlen(path) == 0)
    {
        path = ".";
    }

    linked_list_t *dir_path = unix_tree_traverse(tree, path);
    out_error(dir_path == NULL, out_buffer, "err: directory not found\n");

    file_node_t *dir = (file_node_t *)dir_path->tail->data;
    linked_list_node_t *iter = dir->children->head;
    int index = 0;
    while (iter != NULL)
    {
        file_node_t *child = (file_node_t *)iter->data;
        for (int i = 0; i < strlen(child->name); i++)
        {
            out_buffer[index] = child->name[i];
            index += 1;
        }
        out_buffer[index] = '\n';
        index += 1;

        iter = iter->next;
    }
    out_buffer[index] = '\0';
}

void mkdir(out_buffer_t out_buffer, file_tree_t *tree, const char *path)
{

    out_error(path == NULL || strlen(path) == 0, out_buffer, "err: no directory specified\n");
    linked_list_t *dir_path = unix_tree_traverse_find_closest(tree, path);
    out_error(dir_path == NULL, out_buffer, "err: directory not found\n");

    linked_list_t *path_string_list = linked_list_init();
    
    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");
    while (token != NULL)
    {
        char *token_copy = strdup(token);
        linked_list_insert_tail(path_string_list, token_copy);
        token = strtok(NULL, "/");
    }

    if (strcmp(dir_path->tail->data, path_string_list->tail->data) == 0)
    {
        linked_list_free(path_string_list);
        free(path_copy);
        linked_list_free(dir_path);
        out_error(dir_path == NULL, out_buffer, "err: directory exists");
        return;
    }

    file_node_t *parent = (file_node_t *)dir_path->tail->data;
    file_node_t *new_node = file_tree_add_child(tree, parent, path_string_list->tail->data);
    linked_list_free(path_string_list);
    free(path_copy);
    linked_list_free(dir_path);
    
}

void rmdir(out_buffer_t out_buffer, file_tree_t *tree, const char *path)
{
    out_error(path == NULL || strlen(path) == 0, out_buffer, "err: no directory specified\n");
    linked_list_t *dir_path = unix_tree_traverse(tree, path);
    out_error(dir_path == NULL, out_buffer, "err: directory not found\n");

    file_node_t *node = (file_node_t *)dir_path->tail->data;

    // find parent
    file_node_t *parent = NULL;
    linked_list_node_t *iter = dir_path->head;
    while (iter != NULL && iter->data != node)
    {
        parent = (file_node_t *)iter->data;
        iter = iter->next;
    }

    file_tree_delete_child(tree, parent, node);
}