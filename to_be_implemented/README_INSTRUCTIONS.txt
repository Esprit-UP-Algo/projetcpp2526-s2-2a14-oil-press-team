# Maintenance Management Module - Setup Instructions

Follow these steps to integrate this module into your project:

1.  **File Placement**:
    -   Copy `machine.h` and `machine.cpp` into your project source directory.
    -   Copy `generic_bar_chart.h` into your project source directory.
    -   Copy `maintenance_ui.cpp` into your project source directory.

2.  **Project File (.pro)**:
    Ensure your project file contains the necessary modules:
    ```pro
    QT += core gui sql network printsupport
    ```
    Add the new files to your `SOURCES` and `HEADERS`:
    ```pro
    HEADERS += machine.h generic_bar_chart.h
    SOURCES += machine.cpp maintenance_ui.cpp
    ```

3.  **MainWindow Integration**:
    -   In your `MainWindow` header, ensure you have `#include "maintenance_ui.cpp"` (or declare the `createMaintenancePage` function).
    -   In the `MainWindow` constructor, call the function:
    ```cpp
    QStackedWidget *stackMaintenance;
    stackedWidget->addWidget(createMaintenancePage(stackMaintenance));
    ```

4.  **Database**:
    Ensure the `MACHINE` table is created in your Oracle Database (schema is in the previous file/main README).

5.  **External Config**:
    The email alerts depend on a `config.json` file in the same directory as the executable. It should look like this:
    ```json
    {
      "api_keys": {
        "brevo": "YOUR_API_KEY"
      },
      "email_settings": {
        "admin_email": "admin@example.com",
        "sender_email": "system@example.com"
      }
    }
    ```
