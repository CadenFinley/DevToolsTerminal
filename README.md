# DevToolsTerminal

DevToolsTerminal is a lightweight terminal emulator with integrated OpenAI capabilities. It allows users to execute terminal commands, create and manage shortcuts and multi-command scripts, configure startup commands, and interact with OpenAI's GPT models for enhanced productivity, syntax assistance, and error resolution.

## Features

- **Terminal Integration**: Execute terminal commands directly within the application with proper directory tracking
- **Shortcut Management**: Create, edit, and execute single and multi-command shortcuts for repetitive tasks
- **AI Integration**: Connect with OpenAI's GPT models for AI-assisted development with multiple modes and models
- **Customization**: Configure settings, command prefixes, and startup behaviors
- **Theme Management**: Customize terminal colors and save/load themes
- **Multi-line Input**: Support for entering multi-line commands with full cursor navigation
- **Git Integration**: Automatic Git repository detection with branch display in prompt
- **Data Persistence**: Save and load user preferences, command history, and chat contexts

## Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/cadenfinley/DevToolsTerminal.git
   cd DevToolsTerminal
   ```

2. Build the project:
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Run the application:
   ```sh
   ./DevToolsTerminal
   ```

## Command Format

Commands in DevToolsTerminal follow a consistent format:

1. **Command Prefix**: By default, commands start with `!` (customizable via `!user text commandprefix [char]`)
2. **Command Structure**: `!category subcategory action [arguments]`
   - Example: `!user shortcut add myshortcut ls -la`
3. **Direct Terminal Commands**: Any input without the command prefix is sent directly to the terminal
4. **AI Mode**: When in AI mode (toggled via `!user text defaultentry ai`), input without command prefix is sent to ChatGPT

### Command Types
- **Simple Commands**: Single word commands like `!help`, `!exit`, `!clear`
- **Category Commands**: Multi-level commands like `!user startup add [command]`
- **Shortcut Commands**: Execute defined shortcuts via `!ss [shortcut]` or `!mm [shortcut]`

## Commands Reference

### Basic Commands

| Command | Description |
|---------|-------------|
| `!help` | Display available commands |
| `!exit` | Exit the application and save data |
| `!clear` | Clear screen and terminal cache |
| `!approot` | Navigate to application data directory |
| `!terminal [command]` | Execute terminal command directly |
| `!version` | Display application version information |

### Update Management

| Command | Description |
|---------|-------------|
| `!version` | Display current application version |
| `!checkforupdates` | Manually check for available updates |
| `!user checkforupdates enable` | Enable automatic update checking on startup |
| `!user checkforupdates disable` | Disable automatic update checking on startup |

The application automatically checks for updates on startup (if enabled) and will:
1. Compare local version with latest GitHub release
2. Prompt to download if an update is available
3. Display changelog after installing update

### Shortcuts

| Command | Description |
|---------|-------------|
| `!ss [shortcut]` | Execute a single-command shortcut |
| `!mm [shortcut]` | Execute a multi-command shortcut |

### AI Integration

#### Core AI Commands
| Command | Description |
|---------|-------------|
| `!ai` | Enter AI chat mode |
| `!ai help` | Display AI command help |
| `!ai chat [message]` | Send message to ChatGPT |
| `!aihelp` | Get AI help for recent terminal errors |

#### AI Configuration
| Command | Description |
|---------|-------------|
| `!ai apikey set [KEY]` | Set OpenAI API key |
| `!ai apikey get` | Display current API key |
| `!ai model [model]` | Set AI model (default: gpt-3.5-turbo) |
| `!ai mode [mode]` | Set assistant type (chat, file-search, or code-interpreter) |
| `!ai timeoutflag [seconds]` | Set timeout for AI responses |
| `!ai directory set` | Set directory for AI-generated files to current directory |
| `!ai directory clear` | Reset directory to default (.DTT-Data) |

#### AI File Integration
| Command | Description |
|---------|-------------|
| `!ai file add [file]` | Add file to AI context |
| `!ai file add all` | Add all files in directory to AI context |
| `!ai file remove [file]` | Remove file from AI context |
| `!ai file remove all` | Remove all files from AI context |
| `!ai file active` | List active files in AI context |
| `!ai file available` | List files available in current directory |
| `!ai file refresh` | Refresh active file contents |
| `!ai file clear` | Clear all files from context |

#### AI Chat Management
| Command | Description |
|---------|-------------|
| `!ai chat history clear` | Clear chat history |
| `!ai chat cache enable` | Enable chat cache |
| `!ai chat cache disable` | Disable chat cache |
| `!ai chat cache clear` | Clear chat cache |
| `!ai log` | Log last AI conversation to file |
| `!ai get [key]` | Get specific response data |
| `!ai dump` | Dump complete response data |
| `!ai rejectchanges` | Reject AI-suggested changes |

### User Settings

#### Startup Commands
| Command | Description |
|---------|-------------|
| `!user startup add [command]` | Add command to startup |
| `!user startup remove [command]` | Remove command from startup |
| `!user startup clear` | Clear all startup commands |
| `!user startup enable` | Enable startup commands |
| `!user startup disable` | Disable startup commands |
| `!user startup list` | List all startup commands |
| `!user startup runall` | Run all startup commands now |

#### Shortcuts Management
| Command | Description |
|---------|-------------|
| `!user shortcut add [shortcut] [command]` | Add a shortcut |
| `!user shortcut remove [shortcut]` | Remove a shortcut |
| `!user shortcut clear` | Clear all shortcuts |
| `!user shortcut enable` | Enable shortcuts |
| `!user shortcut disable` | Disable shortcuts |
| `!user shortcut list` | List all shortcuts |

#### Multi-Command Shortcuts
| Command | Description |
|---------|-------------|
| `!user shortcut mm add [name] [cmd1] [cmd2]...` | Add multi-command shortcut |
| `!user shortcut mm remove [name]` | Remove multi-command shortcut |

#### Text and Display Settings
| Command | Description |
|---------|-------------|
| `!user text commandprefix [char]` | Set command prefix character (default: !) |
| `!user text displayfullpath enable` | Show full directory path |
| `!user text displayfullpath disable` | Show shortened directory path |
| `!user text defaultentry ai` | Set default input mode to AI |
| `!user text defaultentry terminal` | Set default input mode to terminal |

#### Color Interface
The terminal features color-coded interface elements:
- Green: AI interactions and successful operations
- Red: Error messages and development mode indicators
- Purple: Application branding and highlights
- Default: Standard command input and output

#### Data Management
| Command | Description |
|---------|-------------|
| `!user data get userdata` | View user settings data |
| `!user data get userhistory` | View command history |
| `!user data get all` | View all user data |
| `!user data clear` | Clear all user data |
| `!user data saveloop enable` | Enable automatic data saving |
| `!user data saveloop disable` | Disable automatic data saving |
| `!user saveonexit enable` | Enable saving data on exit |
| `!user saveonexit disable` | Disable saving data on exit |
| `!user checkforupdates enable` | Enable automatic update checking |
| `!user checkforupdates disable` | Disable automatic update checking |

#### Testing
| Command | Description |
|---------|-------------|
| `!user testing enable` | Enable testing mode |
| `!user testing disable` | Disable testing mode |

### Theme Management

| Command | Description |
|---------|-------------|
| `!theme load [name]` | Load a saved color theme |
| `!theme save [name]` | Save current colors as a theme |

The theme system allows you to customize and persist terminal color schemes:
- Themes are stored in the `.DTT-Data/themes` directory
- Each theme includes settings for all terminal color variables
- Colors can affect prompts, messages, errors, and highlights

### Plugin System

| Command | Description |
|---------|-------------|
| `!plugin list available` | List all available plugins |
| `!plugin list enabled` | List all enabled plugins |
| `!plugin enableall` | Enable all available plugins |
| `!plugin settings` | Show all plugin settings |
| `!plugin [name] enable` | Enable specific plugin |
| `!plugin [name] disable` | Disable specific plugin |
| `!plugin [name] info` | Get information about a plugin |
| `!plugin [name] commands` | List commands provided by a plugin |
| `!plugin [name] settings set [key] [value]` | Change plugin setting |
| `!plugin help` | Show plugin command help |

## OpenAI Integration Details

### Assistant Types
The application supports three OpenAI assistant modes:

1. **chat**: Standard conversational assistant
2. **file-search**: Assistant that analyzes provided files to help with code-related queries
3. **code-interpreter**: Advanced mode that can receive, modify, and create code files

### Code Interpreter Mode
When using the code-interpreter mode, the assistant can:
- Read existing code files
- Suggest changes to code files and automattically apply them (similar to Github Copilot)
- Create new files in the .DTT-Data directory
- Show diffs of the changes made

### File Context
Adding files to the AI context allows the assistant to:
- Reference specific code during conversations
- Understand project structure and dependencies
- Provide more relevant and accurate answers to code-related questions
- Refresh file content with `!ai file refresh` to ensure latest changes are included

## Terminal Features

### Git Integration
The terminal prompt automatically detects Git repositories and shows:
- Current directory name (or full path if enabled)
- Git branch name when inside a repository
- Color-coded terminal information

### Multi-line Editing
The terminal supports full multi-line editing capabilities:
- Arrow key navigation (up, down, left, right)
- Command history navigation
- Backspace handling across lines
- Proper cursor positioning

### Color Themes
The terminal supports customizable color themes that control:
- Command prompts and user input coloring
- System message formatting
- Error and warning highlighting
- Application branding elements

Custom themes can be saved and loaded across sessions for consistent visual experience.

### Cross-Platform Support
The terminal passthrough layer works across:
- Linux (bash)
- macOS (sh)
- Windows (cmd)

## Data Storage

User settings, command history, AI chat history, theme files, and code-interpreter generated files are stored in the `.DTT-Data` directory within your application directory:

- `.USER_DATA.json`: Contains user settings, shortcuts, API keys, and chat cache
- `.USER_COMMAND_HISTORY.txt`: Stores the history of all commands entered
- `themes/`: Directory containing saved color theme files
- Additional directories created by code-interpreter for generated files

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your enhancements.

## License

This project is licensed under the MIT License.

## Author

Caden Finley @ Abilene Christian University (c) 2025
