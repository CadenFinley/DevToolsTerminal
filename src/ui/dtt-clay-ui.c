#include "../include/clay.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

const int FONT_ID_BODY_16 = 0;

Clay_Color COLOR_WHITE = { 230, 230, 230, 255 };
Clay_Color COLOR_LIGHT_GRAY = { 150, 150, 150, 255 };
Clay_Color COLOR_DARK_GRAY = { 40, 40, 40, 255 };
Clay_Color COLOR_TERMINAL_BG = { 24, 24, 27, 255 };
Clay_Color COLOR_TERMINAL_INPUT_BG = { 32, 32, 35, 255 };
Clay_Color COLOR_TERMINAL_PROMPT = { 98, 174, 98, 255 };
Clay_Color COLOR_SELECTION = { 70, 120, 180, 80 };
Clay_Color COLOR_CURSOR = { 230, 230, 230, 255 };

#define MAX_OUTPUT_LINES 1000
#define MAX_INPUT_LENGTH 1024

float Clay_MeasureTextWidth(int fontId, int fontSize, const char* text, int position) {
    if (!text || position <= 0) return 0.0f;
    char temp[MAX_INPUT_LENGTH] = {0};
    strncpy(temp, text, position);
    temp[position] = '\0';
    Clay_String tempString = { .isStaticallyAllocated = true, .length = position, .chars = temp };
    Clay_TextElementConfig config = { 
        .fontId = (uint16_t)fontId, 
        .fontSize = (uint16_t)fontSize 
    };
    Clay_StringSlice slice = { .length = position, .chars = temp, .baseChars = temp };
    Clay_Context* context = Clay_GetCurrentContext();
    void* userData = context ? context->measureTextUserData : NULL;
    Clay_Dimensions dims;
    #ifdef CLAY_WASM
        dims = Clay__MeasureText(slice, &config, userData);
    #else
        if (Clay__MeasureText) {
            dims = Clay__MeasureText(slice, &config, userData);
        } else {
            dims.width = 0;
            dims.height = 0;
        }
    #endif
    return dims.width;
}

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
    bool cursorVisible;
    double lastCursorToggleTime;
    char prompt[64];
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
        .scrollPosition = 0,
        .cursorVisible = true,
        .lastCursorToggleTime = 0
    };
    strcpy(data.prompt, "> ");
    return data;
}

void DevToolsTerminal_UI_AddOutputLine(DevToolsTerminal_UI_Data* data, const char* line) {
    if (data->outputLineCount < MAX_OUTPUT_LINES) {
        strncpy(data->outputLines[data->outputLineCount++], line, 255);
        data->outputLines[data->outputLineCount-1][255] = '\0';
    }
    else {
        for (int i = 0; i < MAX_OUTPUT_LINES - 1; i++) {
            strcpy(data->outputLines[i], data->outputLines[i+1]);
        }
        strncpy(data->outputLines[MAX_OUTPUT_LINES-1], line, 255);
        data->outputLines[MAX_OUTPUT_LINES-1][255] = '\0';
    }
    data->scrollPosition = FLT_MAX;
}

void DevToolsTerminal_UI_UpdateCursor(DevToolsTerminal_UI_Data* data) {
    double currentTime = (double)clock() / CLOCKS_PER_SEC;
    if (currentTime - data->lastCursorToggleTime >= 0.15) {
        data->cursorVisible = !data->cursorVisible;
        data->lastCursorToggleTime = currentTime;
    }
}

Clay_RenderCommandArray DevToolsTerminal_UI_CreateLayout(DevToolsTerminal_UI_Data *data) {
    data->frameArena.offset = 0;
    DevToolsTerminal_UI_UpdateCursor(data);

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    CLAY({ 
        .id = CLAY_ID("OuterContainer"),
        .backgroundColor = COLOR_TERMINAL_BG,
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = layoutExpand,
            .padding = CLAY_PADDING_ALL(4),
            .childGap = 4
        }
    }) {
        CLAY({
            .id = CLAY_ID("ContentContainer"),
            .backgroundColor = COLOR_TERMINAL_BG,
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(1)
                },
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 4
            }
        }) {
            CLAY({
                .id = CLAY_ID("TerminalOutputContainer"),
                .backgroundColor = COLOR_TERMINAL_BG,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(1)
                    },
                    .padding = CLAY_PADDING_ALL(8),
                    .childGap = 4,
                    .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }
                },
                .scroll = { 
                    .vertical = true
                }
            }) {
                CLAY({
                    .id = CLAY_ID("OutputLinesContainer"),
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIT(0)
                        },
                        .childGap = 0
                    }
                }) {
                    for (int i = 0; i < data->outputLineCount; i++) {
                        CLAY({
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(0),
                                    .height = CLAY_SIZING_FIT(0)
                                },
                                .padding = { 0, 0, 0, 0 }
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
                
                CLAY({
                    .id = CLAY_ID("TerminalInputContainer"),
                    .backgroundColor = COLOR_TERMINAL_BG,
                    .layout = {
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIXED(36)
                        },
                        .padding = CLAY_PADDING_ALL(8),
                        .childGap = 4,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
                    }
                }) {
                    CLAY({
                        .id = CLAY_ID("PromptText")
                    }) {
                        CLAY_TEXT(MakeDynamicString(data->prompt), 
                            CLAY_TEXT_CONFIG({
                                .textColor = COLOR_TERMINAL_PROMPT,
                                .fontId = FONT_ID_BODY_16,
                                .fontSize = 16
                            })
                        );
                    }
                    
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
                    
                    if (data->cursorVisible) {
                        Clay_ElementData promptData = Clay_GetElementData(CLAY_ID("PromptText"));
                        Clay_ElementData inputData = Clay_GetElementData(CLAY_ID("InputText"));
                        float cursorX = promptData.boundingBox.width + 13;
                        if (data->cursorPosition > 0) {
                            float inputTextWidth = Clay_MeasureTextWidth(
                                FONT_ID_BODY_16, 
                                16, 
                                data->currentInput, 
                                data->cursorPosition
                            );
                            cursorX += inputTextWidth;
                        }
                        CLAY({
                            .id = CLAY_ID("Cursor"),
                            .backgroundColor = COLOR_CURSOR,
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(2),
                                    .height = CLAY_SIZING_FIXED(18)
                                }
                            },
                            .floating = {
                                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                                .parentId = CLAY_ID("TerminalInputContainer").id,
                                .attachPoints = {
                                    .element = CLAY_ATTACH_POINT_LEFT_CENTER,
                                    .parent = CLAY_ATTACH_POINT_LEFT_CENTER
                                },
                                .offset = {
                                    .x = cursorX,
                                    .y = 0
                                }
                            }
                        }) {}
                    }
                }
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    if (data->scrollPosition == FLT_MAX) {
        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(CLAY_ID("TerminalOutputContainer"));
        if (scrollData.found && scrollData.scrollPosition) {
            float maxScroll = scrollData.contentDimensions.height - scrollData.scrollContainerDimensions.height;
            if (maxScroll > 0) {
                *scrollData.scrollPosition = (Clay_Vector2){ 0, -maxScroll };
            }
        }
    }
    for (int32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }
    return renderCommands;
}

void DevToolsTerminal_UI_InputChar(DevToolsTerminal_UI_Data* data, char c) {
    if (data->cursorPosition < MAX_INPUT_LENGTH - 1) {
        memmove(&data->currentInput[data->cursorPosition + 1], 
                &data->currentInput[data->cursorPosition], 
                strlen(&data->currentInput[data->cursorPosition]) + 1);
        data->currentInput[data->cursorPosition] = c;
        data->cursorPosition++;
        data->cursorVisible = true;
        data->lastCursorToggleTime = (double)clock() / CLOCKS_PER_SEC;
    }
}

void DevToolsTerminal_UI_SetPrompt(DevToolsTerminal_UI_Data* data, const char* prompt) {
    strncpy(data->prompt, prompt, sizeof(data->prompt) - 1);
    data->prompt[sizeof(data->prompt) - 1] = '\0';
}

void DevToolsTerminal_UI_SubmitCommand(DevToolsTerminal_UI_Data* data) {
    char commandLine[MAX_INPUT_LENGTH + sizeof(data->prompt)];
    snprintf(commandLine, sizeof(commandLine), "%s%s", data->prompt, data->currentInput);
    DevToolsTerminal_UI_AddOutputLine(data, commandLine);
    data->currentInput[0] = '\0';
    data->cursorPosition = 0;
    data->cursorVisible = true;
    data->lastCursorToggleTime = (double)clock() / CLOCKS_PER_SEC;
}

void DevToolsTerminal_UI_HandleSpecialKey(DevToolsTerminal_UI_Data* data, int key) {
    switch (key) {
        case 8:
            if (data->cursorPosition > 0) {
                memmove(&data->currentInput[data->cursorPosition - 1], 
                        &data->currentInput[data->cursorPosition], 
                        strlen(&data->currentInput[data->cursorPosition]) + 1);
                data->cursorPosition--;
            }
            break;
        case 37:
            if (data->cursorPosition > 0) {
                data->cursorPosition--;
            }
            break;
        case 39:
            if (data->cursorPosition < strlen(data->currentInput)) {
                data->cursorPosition++;
            }
            break;
    }
    data->cursorVisible = true;
    data->lastCursorToggleTime = (double)clock() / CLOCKS_PER_SEC;
}