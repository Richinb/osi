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
#define DIR_ACCES (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) // 0755
#define FILE_ACCES (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)          // 0644

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

int reverse_file(const char *source_path, const char *dest_path)
{
    int source_fd = open(source_path, O_RDONLY); // read-only
    if (source_fd == -1)
    {
        perror("Failed to open source file");
        return -1;
    }

    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, FILE_ACCES); // write-only, create, rewrite
    if (dest_fd == -1)
    {
        perror("Failed to create destination file");
        close(source_fd);
        return -1;
    }

    off_t file_size = lseek(source_fd, 0, SEEK_END);
    if (file_size == -1)
    {
        perror("Failed to get file size");
        close(source_fd);
        close(dest_fd);
        return -1;
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
            perror("Failed to seek in source file");
            close(source_fd);
            close(dest_fd);
            return -1;
        }

        ssize_t bytes_read = read(source_fd, buffer, chunk_size);
        if (bytes_read == -1)
        {
            perror("Failed to read from source file");
            close(source_fd);
            close(dest_fd);
            return -1;
        }

        // Reverse the chunk in memory
        for (ssize_t i = 0; i < bytes_read / 2; i++)
        {
            char tmp = buffer[i];
            buffer[i] = buffer[bytes_read - 1 - i];
            buffer[bytes_read - 1 - i] = tmp;
        }

        ssize_t bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            perror("Failed to write to destination file");
            close(source_fd);
            close(dest_fd);
            return -1;
        }

        remaining -= bytes_read;
    }

    if (close(dest_fd) != 0)
    {
        perror("Failed to close destination file");
        close(source_fd);
        return -1;
    }

    if (close(source_fd) != 0)
    {
        perror("Failed to close source file");
        return -1;
    }

    return 0;
}

int create_directory(const char *path)
{
    int mkdir_return_value = mkdir(path, DIR_ACCES);
    if (mkdir_return_value == -1)
    {
        if (errno != EEXIST)
        {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

int process_directory_entry(const char *source_path, const char *dest_path, const struct dirent *entry)
{
    char source_entry_path[MAX_PATH_LEN];
    char dest_entry_path[MAX_PATH_LEN];
    int snprintf_res = snprintf(source_entry_path, MAX_PATH_LEN, "%s/%s", source_path, entry->d_name);
    if (snprintf_res >= MAX_PATH_LEN)
    {
        fprintf(stderr, "Error: Path too long: %s/%s\n", source_path, entry->d_name);
        return -1;
    }

    char *reversed_name = reverse_string(entry->d_name);
    if (reversed_name == NULL)
    {
        fprintf(stderr, "Error: Failed to reverse name: %s\n", entry->d_name);
        return -1;
    }
    snprintf_res = snprintf(dest_entry_path, MAX_PATH_LEN, "%s/%s", dest_path, reversed_name);
    if (snprintf_res >= MAX_PATH_LEN)
    {
        fprintf(stderr, "Error: Path too long: %s/%s\n", dest_path, reversed_name);
        free(reversed_name);
        return -1;
    }

    free(reversed_name);

    struct stat st;
    if (lstat(source_entry_path, &st) != 0)
    {
        perror("Failed to get file status");
        return -1;
    }

    if (S_ISDIR(st.st_mode) == 1)
    {
        return copy_directory_recursive(source_entry_path, dest_entry_path);
    }
    else if (S_ISREG(st.st_mode) == 1)
    {
        return reverse_file(source_entry_path, dest_entry_path);
    }

    return 0;
}

int copy_directory_recursive(const char *source_path, const char *dest_path)
{
    struct stat st;
    if (stat(source_path, &st) != 0)
    {
        perror("Failed to access source directory");
        return -1;
    }

    if (S_ISDIR(st.st_mode) != 1)
    {
        fprintf(stderr, "Error: %s is not a directory\n", source_path);
        return -1;
    }

    printf("Processing directory: %s -> %s\n", source_path, dest_path);

    if (create_directory(dest_path) == -1)
    {
        return -1;
    }

    DIR *dir = opendir(source_path);
    if (dir == NULL)
    {
        perror("Failed to open directory");
        return -1;
    }

    int result = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        int on_current_dir = strcmp(entry->d_name, ".");
        int on_upper_dir = strcmp(entry->d_name, "..");
        if (on_current_dir == 0 || on_upper_dir == 0)
        {
            continue;
        }
        int process_return_value = process_directory_entry(source_path, dest_path, entry);
        if (process_return_value == -1)
        {
            result = -1;
            break;
        }
    }

    if (closedir(dir) != 0)
    {
        perror("Failed to close directory");
        result = -1;
    }

    return result;
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

    int ret = copy_directory_recursive(source_dir, reversed_dir_name);
    free(reversed_dir_name);

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}