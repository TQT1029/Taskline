#ifndef THEMEUTILS_H
#define THEMEUTILS_H

#include <QApplication>
#include <QPalette>
#include <QString>

class ThemeUtils {
public:
    static bool isDarkMode() {
        return qApp->palette().color(QPalette::Window).lightness() < 128;
    }

    // Common Colors
    static QString bgApp() { return isDarkMode() ? "#121212" : "#f0f2f5"; }
    static QString bgBar() { return isDarkMode() ? "#1e1e1e" : "#ffffff"; }
    static QString border() { return isDarkMode() ? "#333333" : "#dcdde1"; }
    static QString textMain() { return isDarkMode() ? "#e0e0e0" : "#2c3e50"; }
    static QString textSub() { return isDarkMode() ? "#aaaaaa" : "#57606f"; }
    static QString textDone() { return isDarkMode() ? "#666666" : "#95a5a6"; }
    
    // TaskItem Colors
    static QString itemBg() { return isDarkMode() ? "#1e1e1e" : "#ffffff"; }
    static QString itemBorder() { return isDarkMode() ? "#333333" : "#dcdde1"; }
    static QString itemHoverBg() { return isDarkMode() ? "#252526" : "#f8fbfc"; }
    static QString itemHoverBorder() { return isDarkMode() ? "#3498db" : "#3498db"; }

    // Input / Dialog Colors
    static QString dialogBg() { return isDarkMode() ? "#1e1e1e" : "#f8fafc"; }
    static QString inputBg() { return isDarkMode() ? "#252526" : "#ffffff"; }
    static QString inputBorder() { return isDarkMode() ? "#3c3c3c" : "#cbd5e1"; }
    static QString inputFocus() { return isDarkMode() ? "#3498db" : "#2980b9"; }
    static QString inputDisabledBg() { return isDarkMode() ? "#121212" : "#f1f5f9"; }
    static QString inputDisabledText() { return isDarkMode() ? "#777777" : "#7f8c8d"; }

    // Spinbox Buttons
    static QString spinBtnBg() { return isDarkMode() ? "#333333" : "#f1f5f9"; }
    static QString spinBtnHover() { return isDarkMode() ? "#444444" : "#e2e8f0"; }

    // Combobox dropdown
    static QString comboDropBg() { return isDarkMode() ? "#252526" : "#ffffff"; }

    //CheckBox
    static QString checkBoxBgNormal() { return "#d6d6d6";}
    static QString checkBoxBgChecked() { return isDarkMode()? "#81e6ae":"#5cfaa3";}

    // Button Colors
    static QString btnPrimary() { return isDarkMode() ? "#2980b9" : "#3498db"; }
    static QString btnPrimaryHover() { return isDarkMode() ? "#1c5980" : "#2980b9"; }
    static QString btnPrimaryText() { return "#ffffff"; }

    static QString btnSecondary() { return isDarkMode() ? "#333333" : "#e2e8f0"; }
    static QString btnSecondaryHover() { return isDarkMode() ? "#444444" : "#cbd5e1"; }
    static QString btnSecondaryText() { return isDarkMode() ? "#e0e0e0" : "#475569"; }
};

#endif // THEMEUTILS_H
