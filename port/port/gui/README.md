# Porting GUI Apps to SimpleOS

Guide for creating graphical applications that run on the SimpleOS desktop compositor.

## Architecture

SimpleOS GUI apps communicate with the compositor via shared-memory framebuffers and an IPC message protocol. The widget toolkit (`std.os.gui`) provides high-level abstractions over the raw framebuffer API.

```
+------------------+     +-------------------+     +---------------+
|   Your App       | --> |   Widget Toolkit  | --> |  Compositor   |
|  (app_template)  |     |  (std.os.gui)     |     |  (Wayland-    |
|                  |     |                   |     |   inspired)   |
+------------------+     +-------------------+     +---------------+
                                                         |
                                                   +---------------+
                                                   |  Framebuffer  |
                                                   |  (GPU/VESA)   |
                                                   +---------------+
```

## App Manifest

Every SimpleOS app has an `app.sdn` manifest in its root directory:

```sdn
app:
    name: "My App"
    id: "com.example.myapp"
    version: "1.0.0"
    entry: "main.spl"
    icon: "icon.png"

window:
    title: "My Application"
    width: 800
    height: 600
    resizable: true
    min_width: 400
    min_height: 300

permissions:
    filesystem: read
    network: false
    devices: []
```

## Widget API Overview

### Core Widgets

| Widget | Description |
|--------|------------|
| `Window` | Top-level application window |
| `Box` | Horizontal or vertical layout container |
| `Label` | Static text display |
| `Button` | Clickable button with text/icon |
| `TextInput` | Single-line text entry |
| `TextArea` | Multi-line text editor |
| `Canvas` | Raw drawing surface |
| `Image` | Image display widget |
| `List` | Scrollable item list |
| `ScrollView` | Scrollable container |

### Creating a Window

```simple
use std.os.gui

fn main():
    val app = gui.App(id: "com.example.hello")
    val window = gui.Window(title: "Hello", width: 400, height: 300)

    val label = gui.Label(text: "Hello, SimpleOS!")
    window.add(label)

    app.run(window)
```

### Event Handling

```simple
val button = gui.Button(text: "Click me")
button.on_click(\event:
    print "Button clicked at ({event.x}, {event.y})"
)
```

### Custom Drawing with Canvas

```simple
val canvas = gui.Canvas(width: 320, height: 240)
canvas.on_draw(\ctx:
    ctx.set_color(0xFF0000)  # Red
    ctx.fill_rect(10, 10, 100, 50)
    ctx.set_color(0x000000)  # Black
    ctx.draw_text(20, 30, "Custom drawing")
)
```

## Framebuffer Rendering (Low-Level)

For apps that need direct framebuffer access:

```simple
use std.os.framebuffer

fn main():
    val fb = framebuffer.acquire(width: 800, height: 600)
    # Direct pixel manipulation
    for y in 0..600:
        for x in 0..800:
            fb.set_pixel(x, y, 0x001122)
    fb.flush()
```

## Creating a New App

1. Create a directory under `src/os/apps/`:
   ```
   src/os/apps/myapp/
       app.sdn
       main.spl
       icon.png (optional)
   ```

2. Use the template in `templates/app_template.spl` as a starting point.

3. Build and run:
   ```bash
   bin/simple run src/os/apps/myapp/main.spl
   ```

4. For testing inside QEMU:
   ```bash
   bin/simple run src/os/qemu_runner.spl -- --app myapp
   ```

## Theming

SimpleOS apps inherit the system theme by default. To customize:

```simple
val theme = gui.Theme(
    background: 0xF0F0F0,
    foreground: 0x333333,
    accent: 0x0066CC,
    font_size: 14,
)
app.set_theme(theme)
```
