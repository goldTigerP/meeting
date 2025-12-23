#pragma once

#include <QString>

namespace meeting {
namespace gui {
inline const QString STYLE_SHEET = R"(
        QWidget {
            font-family: "Microsoft YaHei", "SimHei", Arial, sans-serif;
            font-size: 10pt;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #cccccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QPushButton {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 8px 16px;
            text-align: center;
            text-decoration: none;
            font-size: 12px;
            margin: 4px 2px;
            border-radius: 4px;
            min-width: 80px;
        }
        
        QPushButton:hover {
            background-color: #45a049;
        }
        
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        
        QLineEdit {
            border: 2px solid #ddd;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
        }
        
        QLineEdit:focus {
            border-color: #4CAF50;
        }
        
        QListWidget {
            border: 1px solid #ddd;
            border-radius: 4px;
            selection-background-color: #4CAF50;
        }
        
        QTextEdit {
            border: 1px solid #ddd;
            border-radius: 4px;
            background-color: #f9f9f9;
        }
    )";
}
}  // namespace meeting