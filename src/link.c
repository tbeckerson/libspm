#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

// Include necessary headers
#include "globals.h"
#include "libspm.h"
#include "cutils.h"


void create_links(char build_loc[PATH_MAX], char dest_loc[PATH_MAX]) 
{
    // Construct a shell command to list links in the specified directory
    char links_location_cmd[PATH_MAX + 64];
    sprintf(links_location_cmd, "( cd %s && find . -type l | cut -c2- ) ", build_loc);

    // Log the constructed command for debugging
    dbg(2, "Getting links locations with %s ", links_location_cmd);

    // Execute the constructed shell command and store the output in 'res'
    char* res = exec(links_location_cmd);

    // Log the retrieved file locations for debugging
    dbg(3, "Got locations: '%s'", res);

    // Allocate space for links
    char** links;

    // Split the 'res' string into an array of file locations using '\n' as a delimiter
    unsigned int count = splita(res, '\n', &links);

    // Log the count of retrieved locations for debugging
    dbg(2, "Got %d locations", count);

    //this is a variable that stores the command
    char* link_cmd = calloc(count * (PATH_MAX + 64) + 1, sizeof(char));
    for(int i = 0; i < count; i++)
    {
        //allcoaion for variable that store the command for the link
        char* read_link_cmd = calloc(PATH_MAX + 64, sizeof(char));
        char* find_cmd = calloc(PATH_MAX + 64, sizeof(char));
        //reads the file linked by the link
        sprintf(read_link_cmd, "readlink %s/%s", build_loc, links[i]);
        //searches for that file in the build directory
        sprintf(find_cmd, "( cd %s && find . -name %s ", build_loc, exec(read_link_cmd));
        find_cmd[strcspn(find_cmd, "\n")] = 0;
        //creates adds the second half of the command to search for the file
        char* tmp_1 = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(tmp_1, " | cut -c2- ) ");
        strcat(find_cmd, tmp_1);
        //executes the search command to find the target
        char* target = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(target, exec(find_cmd));
        target[strcspn(target, "\n")] = 0;
        //a command to link the link to the target
        char* tmp = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(link_cmd, "ln -sfv %s %s", target, links[i]);
        //will copy the file start to file end in order to cerate the link in the new dir
        char* start = calloc(PATH_MAX + 64, sizeof(char));
        char* end = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(start, "%s/%s", build_loc, target);
        sprintf(end, "%s/%s", dest_loc, target);
        //IMPORTANT: will need to store the targets in an array and go through them, moving them to avoid overwrite
        char* copy_cmd = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(copy_cmd, "cp %s %s", start, end);

        if (!(access(end, F_OK) == 0))
        {
            if (end == NULL) {
                msg(FATAL, "Location is NULL");
            }
            system(copy_cmd);
        }
        else 
        {
            msg(WARNING, "%s is already here, use --overwrite?", end);
            if (OVERWRITE) {
                // Rename the file in the build directory to the destination location
                rename(start, end);
            } else {
                msg(FATAL, "Terminating the program");
            }
        }
        //executes the link command
        dbg(2, "executing %s now", link_cmd);

        system(link_cmd);

        free(read_link_cmd);
        free(find_cmd);
        free(tmp_1);
        free(target);
        free(start);
        free(end);
        free(copy_cmd);
    }
    free(links);
}