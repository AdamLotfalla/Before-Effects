#pragma once
#include <QString>

QString sliderStyle = 
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

QString buttonStyle = 
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