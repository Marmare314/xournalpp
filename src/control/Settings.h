/*
 * Xournal Extended
 *
 * Xournal Settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <libxml/xmlreader.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "../model/Font.h"
#include "Tool.h"
#include "../cfg.h"

#include <map>

enum AttributeType {
	ATTRIBUTE_TYPE_NONE, ATTRIBUTE_TYPE_STRING, ATTRIBUTE_TYPE_INT, ATTRIBUTE_TYPE_DOUBLE, ATTRIBUTE_TYPE_INT_HEX, ATTRIBUTE_TYPE_BOOLEAN,
};

class SAttribute {
public:
	SAttribute();

public:
	String sValue;
	int iValue;
	double dValue;

	AttributeType type;

	String comment;
};

class SElement;

class __RefSElement {
public:
	__RefSElement();
	~__RefSElement();

	void ref();
	void unref();
private:
	std::map<String, SAttribute> attributes;
	std::map<String, SElement> children;

	int refcount;

	friend class SElement;
};

class SElement {
public:
	SElement();
	SElement(const SElement& elem);
	~SElement();

	void operator=(const SElement& elem);

	void clear();

	SElement & child(String name);

	void setIntHex(const String name, const int value);
	void setInt(const String name, const int value);
	void setDouble(const String name, const double value);
	void setBool(const String name, const bool value);
	void setString(const String name, const String value);

	void setComment(const String name, const String comment);


	bool getInt(const String name, int & value);
	bool getDouble(const String name, double & value);
	bool getBool(const String name, bool & value);
	bool getString(const String name, String & value);

	std::map<String, SAttribute> & attributes();
	std::map<String, SElement> & children();

private:
	__RefSElement * element;
};

/**
 * Configuration for Mouse Buttons and Eraser
 */
class ButtonConfig {
public:
	ButtonConfig(ToolType action, int color, ToolSize size, bool shapeRecognizer, bool rouler, EraserType eraserMode);

public:
	ToolType action;
	int color;
	ToolSize size;
	bool shapeRecognizer;
	bool rouler;
	EraserType eraserMode;

	String device;
	bool disableDrawing;
};

class Settings {
public:
	Settings(String filename);
	~Settings();

	bool load();
	void parseData(xmlNodePtr cur, SElement & elem);

	void save();

	/**
	 * Check if there is an XInput device
	 */
	void checkCanXInput();

	/**
	 * Enables / disables extended events
	 */
	//	void updateXEvents();
private:
	void loadDefault();
	void saveTimeout();

	void parseItem(xmlDocPtr doc, xmlNodePtr cur);

	xmlNodePtr savePropertyDouble(const gchar *key, double value, xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar *key, int value, xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar *key, const gchar *value, xmlNodePtr parent);

	void saveData(xmlNodePtr root, String name, SElement & elem);

	void saveButtonConfig();
	void loadButtonConfig();
public:
	// Getter- / Setter
	bool isPresureSensitivity();
	void setPresureSensitivity(gboolean presureSensitivity);

	/**
	 * XInput is enabled by the user
	 */
	bool isXinputEnabled();
	void setXinputEnabled(gboolean useXinput);

	/**
	 * XInput is available
	 */
	bool isXInputAvailable();

	/**
	 * XInput should be used in the application
	 */
	bool isUseXInput();

	/**
	 * The last used font
	 */
	XojFont & getFont();
	void setFont(const XojFont & font);

	/**
	 * The selected Toolbar
	 */
	void setSelectedToolbar(String name);
	String getSelectedToolbar();

	/**
	 * Sets the screen resolution in DPI
	 */
	void setDisplayDpi(int dpi);
	int getDisplayDpi();

	/**
	 * The last saved path
	 */
	void setLastSavePath(String path);
	String getLastSavePath();

	void setMainWndSize(int width, int height);
	void setMainWndMaximized(bool max);
	int getMainWndWidth();
	int getMainWndHeight();
	bool isMainWndMaximized();

	bool isSidebarVisible();
	void setSidebarVisible(bool visible);

	bool isSidebarOnRight();
	void setSidebarOnRight(bool right);

	bool isScrollbarOnLeft();
	void setScrollbarOnLeft(bool right);

	double getWidthMinimumMultiplier();
	double getWidthMaximumMultiplier();

	void setShowTwoPages(bool showTwoPages);
	bool isShowTwoPages();

	bool isAutloadPdfXoj();
	void setAutoloadPdfXoj(bool load);

	int getAutosaveTimeout();
	void setAutosaveTimeout(int autosave);
	bool isAutosaveEnabled();
	void setAutosaveEnabled(bool autosave);

	bool isAllowScrollOutsideThePage();
	void setAllowScrollOutsideThePage(bool outside);

	String getDefaultSaveName();
	void setDefaultSaveName(String name);

	ButtonConfig * getButtonConfig(int id);

	ButtonConfig * getEraserButtonConfig();
	ButtonConfig * getMiddleButtonConfig();
	ButtonConfig * getRightButtonConfig();
	ButtonConfig * getTouchButtonConfig();

	String getFullscreenHideElements();
	void setFullscreenHideElements(String elements);

	String getPresentationHideElements();
	void setPresentationHideElements(String elements);


	PageInsertType getPageInsertType();
	void setPageInsertType(PageInsertType type);

	int getPageBackgroundColor();
	void setPageBackgroundColor(int color);

	int getSelectionColor();
	void setSelectionColor(int color);

public:
	// Custom settings
	SElement & getElement(String name);

	/**
	 * Call this after you have done all custom settings changes
	 */
	void customSettingsChanged();

private:
	Settings(const Settings& settings) {
	}
	Settings & operator=(const Settings & settings) {
	}

private:
	bool saved;
	gint timeoutId;

	/**
	 * The config filename
	 */
	String filename;

private:
	// Settings
	/**
	 * The settings tree
	 */
	std::map<String, SElement> data;

	/**
	 * Use XInput
	 */
	bool useXinput;

	/**
	 * If there is an XInput device available
	 */
	bool canXIput;

	/**
	 * Use pen pressure to control stroke width?
	 */
	bool presureSensitivity;

	/**
	 * If the sidebar is visible
	 */
	bool showSidebar;

	/**
	 * If the sidebar is on the right
	 */
	bool sidebarOnRight;

	/**
	 * The selected Toolbar name
	 */
	String selectedToolbar;

	/**
	 * The last saved folder
	 */
	String lastSavePath;

	/**
	 * The last used font
	 */
	XojFont font;

	/**
	 * The display resolution, in pixels per inch
	 */
	gint displayDpi;

	/**
	 * If the window is maximized
	 */
	bool maximized;

	/**
	 * The Main window size
	 */
	int mainWndWidth;
	int mainWndHeight;

	/**
	 * Show Scrollbar on left
	 */
	bool scrollbarOnLeft;

	/**
	 * Displays two pages
	 */
	bool showTwoPages;

	/**
	 * Automatically load filename.pdf.xoj instead of filename.pdf (true/false)
	 */
	bool autoloadPdfXoj;

	/**
	 * Minimum width multiplier
	 */
	double widthMinimumMultiplier;

	/**
	 * maximum width multiplier
	 */
	double widthMaximumMultiplier;

	/**
	 * automatically save documents for crash recovery each x minutes
	 */
	int autosaveTimeout;
	bool autosaveEnabled;

	/**
	 * allow scroll outside the page
	 */
	bool allowScrollOutsideThePage;

	/**
	 * Default name if you save a new document
	 */
	String defaultSaveName;

	/**
	 * The button config
	 *
	 * 0: eraser
	 * 1: middle button
	 * 2: right button
	 * 3: touch screen
	 */
	ButtonConfig * buttonConfig[4];

	/**
	 * Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)
	 */
	String fullscreenHideElements;
	String presentationHideElements;

	/**
	 * If you insert a page, which type will be selected? Plain, Lined, Copy current page...
	 */
	PageInsertType pageInsertType;

	/**
	 * The background color of a new inserted page
	 */
	int pageBackgroundColor;

	/**
	 * The color to draw borders on selected elements (Page, insert image selection etc.)
	 */
	int selectionColor;
};

#endif /* __SETTINGS_H__ */
