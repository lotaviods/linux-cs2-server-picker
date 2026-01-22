# CS2 Server Picker (Qt Version)

A desktop application for managing Counter-Strike 2 game servers, built with Qt and C++.

## Features

- **Server Discovery**: Automatically fetches server information from Steam's API
- **Latency Testing**: Ping servers to check response times
- **Firewall Management**: Block or unblock servers using system firewall rules
- **Server Clustering**: Group servers by region for easier management
- **Cross-Platform**: Built with Qt for potential cross-platform support

## Requirements

### Dependencies
- Qt6 (Core, Widgets, Network, Concurrent, Test)
- CMake 3.16 or higher
- C++17 compatible compiler
- Linux (currently supports iptables firewall management)

### System Libraries
- libqt6core6
- libqt6widgets6
- libqt6network6
- libqt6concurrent6
- iptables (for firewall management)
- ping (for latency testing)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

### Interface

- **Refresh Servers**: Fetch latest server data from Steam
- **Ping All**: Test latency to all servers
- **Block/Unblock**: Select servers and modify firewall rules
- **Toggle Cluster**: Switch between individual and grouped server view

## Architecture

- **MainWindow**: Qt-based GUI with server table and controls
- **ServerService**: Handles server data fetching and management
- **PingService**: Asynchronous ping operations
- **FirewallService**: Platform-specific firewall rule management

## FAQ

1. **How it works, will I get banned?!**

    The app does not modify any game or system files, I can assure you are safe from being banned when using the app as long as you do not download from untrusted sources. It will add necessary firewall policies to block game server relay IP addresses from being accessed by your network thus skipping them in-game when finding a match.

2. **Not being routed to lowest ping server or not working on your location?**

    Due to the fact that we can only access and block IP relay addresses from Valve's network points around the world rather than the game's actual server IP addresses directly, which are not exposed publicly, either your connection got relayed to the nearest available server due to how Steam Datagram relay works or your location might be a factor.
    Re-routing can also happen anytime, even mid-game. One of the best ways to test it out is to block low-ping servers and leave out high-ping servers that are far from your current region. If your ping is high in-game, then you are being routed properly, and the blocked IP relays are not able to re-route you to a nearby server.
    Some solutions that might help out but are not guaranteed: turning off any VPN, disabling third-party firewalls and let ufw manage the firewall.
    ISP-related issues, such as bad routing or high ping, are out of scope and control since the app only adds firewall entries. Please contact your ISP instead.

3. **Why it requires admin permission on execution?**

    This is due to how Linux requires elevated execution when adding the necessary firewall policies. If the app is running in normal mode, it will not be able to do its operations and will throw errors.

## Credits

Inspired by the original CS2 Server Picker project: [https://github.com/FN-FAL113/cs2-server-picker](https://github.com/FN-FAL113/cs2-server-picker)

This is a Qt-based desktop port of the original C# ASP.NET Core web application.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
