#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#define BUFFER_SIZE 4096
#define MAX_PATH_LEN 4096

int copy_directory_recursive(const char *source_path, const char *dest_path);

char *reverse_string(const char *str)
{
    if (str == NULL)
    {
        fprintf(stderr, "Error: NULL string passed to reverse_string\n");
        return NULL;
    }

    size_t len = strlen(str);
    char *reversed = malloc(len + 1);
    if (reversed == NULL)
    {
        perror("malloc failed in reverse_string");
        return NULL;
    }

    for (size_t i = 0; i < len; i++)
    {
        reversed[i] = str[len - 1 - i];
    }
    reversed[len] = '\0';

    return reversed;
}

int init_files(const char *source_path, const char *dest_path, int *source_fd, int *dest_fd)
{
    *source_fd = open(source_path, O_RDONLY); // read-only
    if (*source_fd == -1)
    {
        perror("Failed to open source file");
        return EXIT_FAILURE;
    }
    struct stat src_stat;
    int return_code = stat(source_path, &src_stat);
    if (return_code == -1)
    {
        perror("Error getting file info");
        return EXIT_FAILURE;
    }
    mode_t mode = src_stat.st_mode;
    *dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, mode); // write-only, create, rewrite
    if (*dest_fd == -1)
    {
        perror("Failed to create destination file");
        close(*source_fd);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void error_close(const char *error, int *source_fd, int *dest_fd)
{
    perror(error);
    close(*source_fd);
    close(*dest_fd);
}

void reverse_buffer(char buffer[BUFFER_SIZE], ssize_t bytes_read)
{
    for (ssize_t i = 0; i < bytes_read / 2; i++)
    {
        char tmp = buffer[i];
        buffer[i] = buffer[bytes_read - 1 - i];
        buffer[bytes_read - 1 - i] = tmp;
    }
}

int reverse_file(const char *source_path, const char *dest_path)
{
    int source_fd = 0;
    int dest_fd = 0;
    int return_code = init_files(source_path, dest_path, &source_fd, &dest_fd);
    if (return_code == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    off_t file_size = lseek(source_fd, 0, SEEK_END);
    if (file_size == -1)
    {
        error_close("Failed to get file size", &source_fd, &dest_fd);
        return EXIT_FAILURE;
    }
    char buffer[BUFFER_SIZE];
    off_t remaining = file_size;
    while (remaining > 0)
    {
        size_t chunk_size = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : (size_t)remaining;
        off_t read_pos = remaining - chunk_size;
        int position_status = lseek(source_fd, read_pos, SEEK_SET);
        if (position_status == -1)
        {
            error_close("Failed to seek in source file", &source_fd, &dest_fd);
            return EXIT_FAILURE;
        }

        ssize_t bytes_read = read(source_fd, buffer, chunk_size);
        if (bytes_read == -1)
        {
            error_close("Failed to read from source file", &source_fd, &dest_fd);
            return EXIT_FAILURE;
        }
        reverse_buffer(buffer, bytes_read);

        ssize_t bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            error_close("Failed to write to destination file", &source_fd, &dest_fd);
            return EXIT_FAILURE;
        }
        remaining -= bytes_read;
    }
    close(dest_fd);
    close(source_fd);
    return EXIT_SUCCESS;
}

int create_directory(const char *path, const char *src)
{
    struct stat src_stat;
    int return_code = stat(src, &src_stat);
    if (return_code == -1)
    {
        perror("Error getting source directory info");
        return EXIT_FAILURE;
    }
    mode_t mode = src_stat.st_mode;
    return_code = mkdir(path, mode);
    if (return_code == -1)
    {
        perror("Failed to create directory");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int process_directory_entry(const char *source_path, const char *dest_path, const struct dirent *entry)
{
    char source_entry_path[MAX_PATH_LEN];
    char dest_entry_path[MAX_PATH_LEN];
    int snprintf_res = snprintf(source_entry_path, MAX_PATH_LEN, "%s/%s", source_path, entry->d_name);
    if (snprintf_res >= MAX_PATH_LEN)
    {
        fprintf(stderr, "Error: Path too long: %s/%s\n", source_path, entry->d_name);
        return EXIT_FAILURE;
    }

    char *reversed_name = reverse_string(entry->d_name);
    if (reversed_name == NULL)
    {
        fprintf(stderr, "Error: Failed to reverse name: %s\n", entry->d_name);
        return EXIT_FAILURE;
    }
    snprintf_res = snprintf(dest_entry_path, MAX_PATH_LEN, "%s/%s", dest_path, reversed_name);
    if (snprintf_res >= MAX_PATH_LEN)
    {
        fprintf(stderr, "Error: Path too long: %s/%s\n", dest_path, reversed_name);
        free(reversed_name);
        return EXIT_FAILURE;
    }

    free(reversed_name);

    struct stat path_stat;
    int return_code = lstat(source_entry_path, &path_stat);
    if (return_code != 0)
    {
        perror("Failed to get file status");
        return EXIT_FAILURE;
    }

    mode_t file_type = path_stat.st_mode;
    if (S_ISDIR(file_type))
    {
        return_code = copy_directory_recursive(source_entry_path, dest_entry_path);
    }
    if (S_ISREG(file_type))
    {
        return_code = reverse_file(source_entry_path, dest_entry_path);
    }

    return return_code;
}

int copy_directory_recursive(const char *source_path, const char *dest_path)
{
    struct stat path_stat;
    int return_code = stat(source_path, &path_stat);
    if (return_code != 0)
    {
        perror("Failed to access source directory");
        return EXIT_FAILURE;
    }
    return_code = create_directory(dest_path, source_path);
    if (return_code == -1)
    {
        return EXIT_FAILURE;
    }

    DIR *src_dir = opendir(source_path);
    if (src_dir == NULL)
    {
        perror("Failed to open directory");
        return EXIT_FAILURE;
    }

    struct dirent *src_dir_entry;
    while ((src_dir_entry = readdir(src_dir)) != NULL)
    {
        int on_current_dir = strcmp(src_dir_entry->d_name, ".");
        int on_upper_dir = strcmp(src_dir_entry->d_name, "..");
        if (on_current_dir == 0 || on_upper_dir == 0)
        {
            continue;
        }
        int return_code = process_directory_entry(source_path, dest_path, src_dir_entry);
        if (return_code == EXIT_FAILURE)
        {
            break;
        }
    }

    closedir(src_dir);
    return return_code;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *source_dir = argv[1];
    char *base_dir = basename(source_dir);
    if (base_dir == NULL)
    {
        perror("basename failed");
        return EXIT_FAILURE;
    }
    char *reversed_dir_name = reverse_string(base_dir);
    if (reversed_dir_name == NULL)
    {
        fprintf(stderr, "Error: Failed to reverse directory name\n");
        return EXIT_FAILURE;
    }

    int return_code = copy_directory_recursive(source_dir, reversed_dir_name);
    free(reversed_dir_name);

    return return_code;
}
