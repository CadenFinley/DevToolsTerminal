#include "../include/clay.h"
#include <stdlib.h>
#include <string.h>

const int FONT_ID_BODY_16 = 0;
Clay_Color COLOR_WHITE = { 255, 255, 255, 255};
Clay_Color COLOR_LIGHT_GRAY = { 200, 200, 200, 255 };
Clay_Color COLOR_DARK_GRAY = { 50, 50, 50, 255 };
Clay_Color COLOR_TERMINAL_BG = { 30, 30, 30, 255 };
Clay_Color COLOR_TERMINAL_INPUT_BG = { 40, 40, 40, 255 };
Clay_Color COLOR_TERMINAL_PROMPT = { 100, 255, 100, 255 };

#define MAX_OUTPUT_LINES 100
#define MAX_INPUT_LENGTH 256

typedef struct {
    intptr_t offset;
    intptr_t memory;
} DevToolsTerminal_UI_Arena;

typedef struct {
    float yOffset;
    DevToolsTerminal_UI_Arena frameArena;
    
    char outputLines[MAX_OUTPUT_LINES][256];
    int outputLineCount;
    char currentInput[MAX_INPUT_LENGTH];
    int cursorPosition;
    float scrollPosition;
} DevToolsTerminal_UI_Data;

Clay_String MakeDynamicString(const char* text) {
    Clay_String str;
    str.isStaticallyAllocated = false;
    str.length = strlen(text);
    str.chars = text;
    return str;
}

DevToolsTerminal_UI_Data DevToolsTerminal_UI_Initialize() {
    DevToolsTerminal_UI_Data data = {
        .frameArena = { .memory = (intptr_t)malloc(1024) },
        .outputLineCount = 0,
        .cursorPosition = 0,
        .scrollPosition = 0
    };
    return data;
}

void DevToolsTerminal_UI_AddOutputLine(DevToolsTerminal_UI_Data* data, const char* line) {
    if (data->outputLineCount < MAX_OUTPUT_LINES) {
        strncpy(data->outputLines[data->outputLineCount++], line, 255);
        data->outputLines[data->outputLineCount-1][255] = '\0';
    }
    else {
        // Scroll the buffer up
        for (int i = 0; i < MAX_OUTPUT_LINES - 1; i++) {
            strcpy(data->outputLines[i], data->outputLines[i+1]);
        }
        strncpy(data->outputLines[MAX_OUTPUT_LINES-1], line, 255);
        data->outputLines[MAX_OUTPUT_LINES-1][255] = '\0';
    }
}

Clay_RenderCommandArray DevToolsTerminal_UI_CreateLayout(DevToolsTerminal_UI_Data *data) {
    data->frameArena.offset = 0;

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    // Main container
    CLAY({ 
        .id = CLAY_ID("OuterContainer"),
        .backgroundColor = COLOR_DARK_GRAY,
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(8),
            .childGap = 8
        }
    }) {
        // Output area (scrollable terminal text)
        CLAY({
            .id = CLAY_ID("TerminalOutputContainer"),
            .backgroundColor = COLOR_TERMINAL_BG,
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(1, 100)
                },
                .padding = CLAY_PADDING_ALL(8),
                .childGap = 2
            },
            .scroll = { .vertical = true }
        }) {
            // Render each line of terminal output
            for (int i = 0; i < data->outputLineCount; i++) {
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIT(0)
                        }
                    }
                }) {
                    Clay_String dynamicString = MakeDynamicString(data->outputLines[i]);
                    CLAY_TEXT(dynamicString, 
                        CLAY_TEXT_CONFIG({
                            .textColor = COLOR_WHITE,
                            .fontId = FONT_ID_BODY_16,
                            .fontSize = 16
                        })
                    );
                }
            }
        }
        
        // Input area
        CLAY({
            .id = CLAY_ID("TerminalInputContainer"),
            .backgroundColor = COLOR_TERMINAL_INPUT_BG,
            .layout = {
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIXED(40)
                },
                .padding = CLAY_PADDING_ALL(8),
                .childGap = 8,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
            }
        }) {
            // Prompt
            CLAY_TEXT(CLAY_STRING("> "), 
                CLAY_TEXT_CONFIG({
                    .textColor = COLOR_TERMINAL_PROMPT,
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 16
                })
            );
            
            // Input text
            CLAY({
                .id = CLAY_ID("InputText"),
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_FIT(0)
                    }
                }
            }) {
                Clay_String dynamicString = MakeDynamicString(data->currentInput);
                CLAY_TEXT(dynamicString, 
                    CLAY_TEXT_CONFIG({
                        .textColor = COLOR_WHITE,
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 16
                    })
                );
            }
            
            // Cursor
            CLAY({
                .id = CLAY_ID("Cursor"),
                .backgroundColor = COLOR_LIGHT_GRAY,
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(2),
                        .height = CLAY_SIZING_FIXED(20)
                    }
                }
            }) {}
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }
    return renderCommands;
}

// Functions that will be needed to handle input in main.cpp:

// Call this when a character is typed
void DevToolsTerminal_UI_InputChar(DevToolsTerminal_UI_Data* data, char c) {
    if (data->cursorPosition < MAX_INPUT_LENGTH - 1) {
        // Make space for the new character
        memmove(&data->currentInput[data->cursorPosition + 1], 
                &data->currentInput[data->cursorPosition], 
                strlen(&data->currentInput[data->cursorPosition]) + 1);
                
        // Insert the character
        data->currentInput[data->cursorPosition] = c;
        data->cursorPosition++;
    }
}

// Call this when Enter is pressed
void DevToolsTerminal_UI_SubmitCommand(DevToolsTerminal_UI_Data* data) {
    char commandLine[MAX_INPUT_LENGTH + 3];
    snprintf(commandLine, sizeof(commandLine), "> %s", data->currentInput);
    
    // Add the command to output
    DevToolsTerminal_UI_AddOutputLine(data, commandLine);
    
    // Process command if needed
    // ...
    
    // Clear the input
    data->currentInput[0] = '\0';
    data->cursorPosition = 0;
}

// Handle special keys
void DevToolsTerminal_UI_HandleSpecialKey(DevToolsTerminal_UI_Data* data, int key) {
    switch (key) {
        case 8: // Backspace
            if (data->cursorPosition > 0) {
                memmove(&data->currentInput[data->cursorPosition - 1], 
                        &data->currentInput[data->cursorPosition], 
                        strlen(&data->currentInput[data->cursorPosition]) + 1);
                data->cursorPosition--;
            }
            break;
        
        case 37: // Left arrow
            if (data->cursorPosition > 0) {
                data->cursorPosition--;
            }
            break;
            
        case 39: // Right arrow
            if (data->cursorPosition < strlen(data->currentInput)) {
                data->cursorPosition++;
            }
            break;
    }
}