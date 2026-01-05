#ifndef COMMON_WIDGET_STYLES_H
    #include <QString>
    #define COMMON_WIDGET_STYLES_H



inline QString sliderStyle = 
    "QSlider {"
    "    background: transparent;"  // Transparent background
    "}"
    "QSlider::groove:horizontal {"
    "    border: none;"
    "    height: 4px;"
    "    background: #404040;"  // Dark track
    "    border-radius: 2px;"
    "}"
    "QSlider::sub-page:horizontal {"
    "    background: white;"    // White progress
    "    border-radius: 2px;"
    "}"
    "QSlider::add-page:horizontal {"
    "    background: #404040;"  // Dark unfilled portion
    "    border-radius: 2px;"
    "}"
    "QSlider::handle:horizontal {"
    "    background: white;"
    "    border: 2px solid #2E2E2E;"  // Border matches toolbar
    "    width: 6px;"
    "    height: 6px;"
    "    margin: -6px 0;"
    "    border-radius: 3px;"
    "}"
    "QSlider::handle:horizontal:hover {"
    "    border: 2px solid #3E3E3E;"
    "}"
    "QSlider::handle:horizontal:pressed {"
    "    border: 2px solid #4E4E4E;"
    "}";

inline QString buttonStyle = 
    "QToolButton {"
    "    background-color: #2E2E2E;"  // Match toolbar exactly
    "    border: none;"
    "    padding: 4px;"
    "    margin: 0px;"
    "}"
    "QToolButton:pressed {"
    "    background-color: #4E4E4E;"  // Much lighter for visibility
    "    border-radius: 3px;"
    "}"
    "QToolButton:hover:!pressed {"
    "    background-color: #3A3A3A;"  // Subtle hover
    "    border-radius: 3px;"
    "}";

inline QString toolBarButtonStyle = 
    "QToolButton {"
    "    background-color: #2E2E2E;"  // Match toolbar exactly
    "    border: none;"
    "    padding: 4px;"
    "    margin: 0px;"
    "}"
    "QToolButton:pressed {"
    "    background-color: #4E4E4E;"  // Much lighter for visibility
    "    border-radius: 3px;"
    "}"
    "QToolButton:hover:!pressed {"
    "    background-color: #3A3A3A;"  // Subtle hover
    "    border-radius: 3px;"
    "}"
    "QToolButton:checked {"
    "    background-color: #4E4E4E;"  // Same as pressed state
    "    border: 1px solid #7BC7B0;"  // Border with your color
    "    border-radius: 3px;"
    "    padding: 2px;"  // Adjust padding to account for border
    "}"
    "QToolButton:checked:hover {"
    "    background-color: #5E5E5E;"  // Even lighter on hover when checked
    "    border: 2px solid #7BC7B0;"
    "    border-radius: 3px;"
    "}"
    "QToolButton:checked:pressed {"
    "    background-color: #6E6E6E;"  // Lightest when pressed while checked
    "    border: 1px solid #7BC7B0;"
    "    border-radius: 3px;"
    "}";

#endif