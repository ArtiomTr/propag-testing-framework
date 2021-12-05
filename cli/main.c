// Author: Artiom Tretjakovas
// Description: Entrypoint for the GcLaDOS cli.

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "args.h"
#include "gclados.h"
#include "generate.h"
#include "help.h"
#include "run.h"

// Structure, that is used for saving general (global) CLI arguments.
typedef struct {
    bool colors;
} GeneralArguments;

// Function for parsing general arguments.
GeneralArguments parseGeneralArguments(int *argc, char *argv[]) {
    Argument generalArguments[] = {
            createBoolArgument("colors", "Enables / disables stdout coloring with ANSI escape codes."),
    };

    void **values = parseArguments(generalArguments, sizeof(generalArguments) / sizeof(Argument), argc, argv);

    GeneralArguments generalArgumentValues;

    if(values[0] != NULL) {
        generalArgumentValues.colors = *((bool *) values[0]);
    } else {
        // By default, colored output is enabled.
        generalArgumentValues.colors = true;
    }

    return generalArgumentValues;
}

int main(int argc, char *argv[]) {
    // If no CLI arguments specified, printing general help.
    if(argc == 1) {
        printGeneralHelp();

        return EXIT_FAILURE;
    }

    char *commandSlug = argv[1];

    argc -= 2;
    argv += 2;

    GeneralArguments arguments = parseGeneralArguments(&argc, argv);
    gcladosColors.setColorsSupported(arguments.colors);

    Command runCmd = createRunCommand();
    Command generateCmd = createGenerateCommand();
    Command helpCmd = createHelpCommand();
    Command allCommands[] = {
            helpCmd,
            runCmd,
            generateCmd,
    };
    Command *currentCommand = NULL;

    int commandCount = sizeof(allCommands) / sizeof(Command);

    for(int i = 0; i < commandCount; ++i) {
        if(strcmp(allCommands[i].slug, commandSlug) == 0) {
            currentCommand = &allCommands[i];
            break;
        }
    }

    // Unknown command got.
    if(currentCommand == NULL) {
        char buffer[100];
        sprintf(buffer,
                "Unknown command received \"%s\". "
                "Run \"gclados help\" to see list of all available commands",
                commandSlug);
        gcladosPanic(buffer, EXIT_FAILURE);
    }

    void *opts = currentCommand->parseArgs(argc, (const char **) argv);

    if(opts != NULL) {
        return currentCommand->execute(opts);
    } else {
        // Failed to parse command options.
        return EXIT_FAILURE;
    }
}
