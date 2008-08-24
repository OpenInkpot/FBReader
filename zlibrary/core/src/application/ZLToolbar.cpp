/*
 * Copyright (C) 2004-2008 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdlib.h>

#include <ZLApplication.h>
#include <ZLXMLReader.h>
#include <ZLResource.h>
#include <ZLibrary.h>
#include <ZLPopupData.h>
#include "ZLToolbar.h"
#include "ZLApplicationWindow.h"

#include <optionEntries/ZLStringInfoEntry.h>

class ZLToolbarCreator : public ZLXMLReader {

public:
	ZLToolbarCreator(ZLToolbar &toolbar);
	void startElementHandler(const char *tag, const char **attributes);

private:
	ZLToolbar &myToolbar;
};

void ZLApplication::createToolbar(int index) {
	toolbar(index);
	std::string fileName = ZLibrary::DefaultFilesPathPrefix();
	const bool isWindowToolbar = index == ZLApplicationWindow::WINDOW_TOOLBAR;
	fileName += isWindowToolbar ? "toolbar.xml" : "fullscreen_toolbar.xml";
	ZLToolbarCreator(isWindowToolbar ? *myToolbar : *myFullscreenToolbar).readDocument(fileName);
}

const ZLToolbar &ZLApplication::toolbar(int index) const {
	if (index == ZLApplicationWindow::WINDOW_TOOLBAR) {
		if (myToolbar.isNull()) {
			myToolbar = new ZLToolbar();
		}
		return *myToolbar;
	} else {
		if (myFullscreenToolbar.isNull()) {
			myFullscreenToolbar = new ZLToolbar();
		}
		return *myFullscreenToolbar;
	}
}

void ZLApplication::registerPopupData(const std::string &actionId, shared_ptr<ZLPopupData> popupData) {
	toolbar(ZLApplicationWindow::WINDOW_TOOLBAR);
	myToolbar->registerPopupData(actionId, popupData);
}

void ZLToolbar::registerPopupData(const std::string &actionId, shared_ptr<ZLPopupData> popupData) {
	myPopupDataMap[actionId] = popupData;
}

ZLToolbarCreator::ZLToolbarCreator(ZLToolbar &toolbar) : myToolbar(toolbar) {
}

void ZLToolbarCreator::startElementHandler(const char *tag, const char **attributes) {
	static const std::string BUTTON = "button";
	static const std::string MENU_BUTTON = "menuButton";
	static const std::string TOGGLE_BUTTON = "toggleButton";
	static const std::string TEXT_FIELD = "textField";
	static const std::string SEPARATOR = "separator";
	if (BUTTON == tag) {
		const char *id = attributeValue(attributes, "id");
		if (id != 0) {
			myToolbar.addPlainButton(id);
		}
	} else if (MENU_BUTTON == tag) {
		const char *id = attributeValue(attributes, "id");
		if (id != 0) {
			myToolbar.addMenuButton(id);
		}
	} else if (TOGGLE_BUTTON == tag) {
		const char *id = attributeValue(attributes, "id");
		const char *groupId = attributeValue(attributes, "group");
		const char *isDefault = attributeValue(attributes, "default");
		if ((id != 0) && (groupId != 0)) {
			ZLToolbar::ToggleButtonItem &button = myToolbar.addToggleButton(id, groupId);
			if (isDefault != 0) {
				button.press();
			}
		}
	} else if (TEXT_FIELD == tag) {
		const char *id = attributeValue(attributes, "id");
		const char *parameterId = attributeValue(attributes, "parameterId");
		const char *maxWidth = attributeValue(attributes, "maxWidth");
		const char *symbolSet = attributeValue(attributes, "symbols");
		if ((id != 0) && (parameterId != 0) && (maxWidth != 0)) {
			int nMaxWidth = atoi(maxWidth);
			ZLToolbar::TextFieldItem::SymbolSet sSet =
				((symbolSet != 0) && (std::string(symbolSet) == "digits")) ?
					ZLToolbar::TextFieldItem::SET_DIGITS :
					ZLToolbar::TextFieldItem::SET_ANY;
			if (nMaxWidth > 0) {
				myToolbar.addTextField(id, parameterId, nMaxWidth, sSet);
			}
		}
	} else if (SEPARATOR == tag) {
		myToolbar.addSeparator();
	}
}

const ZLToolbar::ItemVector &ZLToolbar::items() const {
	return myItems;
}

ZLToolbar::Item::Item(const ZLToolbar &toolbar) : myToolbar(toolbar) {
}

ZLToolbar::Item::~Item() {
}

const ZLToolbar &ZLToolbar::Item::toolbar() const {
	return myToolbar;
}

ZLToolbar::ActionItem::ActionItem(const ZLToolbar &toolbar, const std::string &actionId, const ZLResource &tooltip) : Item(toolbar), myActionId(actionId), myTooltip(tooltip) {
}

const std::string &ZLToolbar::ActionItem::actionId() const {
	return myActionId;
}

const std::string &ZLToolbar::ActionItem::tooltip() const {
	if (!myTooltip.hasValue()) {
		static const std::string EMPTY;
		return EMPTY;
	}
	return myTooltip.value();
}

ZLToolbar::AbstractButtonItem::AbstractButtonItem(const ZLToolbar &toolbar, const std::string &actionId, const ZLResource &tooltip) : ActionItem(toolbar, actionId, tooltip) {
}

ZLToolbar::PlainButtonItem::PlainButtonItem(const ZLToolbar &toolbar, const std::string &actionId, const ZLResource &tooltip) : AbstractButtonItem(toolbar, actionId, tooltip) {
}

ZLToolbar::MenuButtonItem::MenuButtonItem(const ZLToolbar &toolbar, const std::string &actionId, const ZLResource &tooltip) : AbstractButtonItem(toolbar, actionId, tooltip) {
}

const std::string &ZLToolbar::MenuButtonItem::popupTooltip() const {
	const ZLResource &popupResource = myTooltip["popup"];
	if (!popupResource.hasValue()) {
		static const std::string EMPTY;
		return EMPTY;
	}
	return popupResource.value();
}

shared_ptr<ZLPopupData> ZLToolbar::MenuButtonItem::popupData() const {
	std::map<std::string,shared_ptr<ZLPopupData> >::const_iterator it = toolbar().myPopupDataMap.find(actionId());
	return (it == toolbar().myPopupDataMap.end()) ? 0 : it->second;
}

ZLToolbar::ToggleButtonItem::ToggleButtonItem(const ZLToolbar &toolbar, const std::string &actionId, ButtonGroup &group, const ZLResource &tooltip) : AbstractButtonItem(toolbar, actionId, tooltip), myGroup(group) {
	myGroup.Items.insert(this);
}

const std::string &ZLToolbar::AbstractButtonItem::iconName() const {
	return actionId();
}

ZLToolbar::ButtonGroup &ZLToolbar::ToggleButtonItem::buttonGroup() {
	return myGroup;
}

void ZLToolbar::ToggleButtonItem::press() {
	myGroup.press(this);
}

bool ZLToolbar::ToggleButtonItem::isPressed() const {
	return this == myGroup.PressedItem;
}

ZLToolbar::ButtonGroup &ZLToolbar::getButtonGroup(const std::string &id) {
	shared_ptr<ButtonGroup> group = myButtonGroups[id];
	if (group.isNull()) {
		group = new ButtonGroup();
		myButtonGroups[id] = group;
	}
	return *group;
}

ZLToolbar::ZLToolbar() : myResource(ZLResource::resource("toolbar")) {
}

void ZLToolbar::addPlainButton(const std::string &actionId) {
	myItems.push_back(
		new PlainButtonItem(*this, actionId, myResource[ZLResourceKey(actionId)])
	);
}

void ZLToolbar::addMenuButton(const std::string &actionId) {
	myItems.push_back(
		new MenuButtonItem(*this, actionId, myResource[ZLResourceKey(actionId)])
	);
}

ZLToolbar::ToggleButtonItem &ZLToolbar::addToggleButton(const std::string &actionId, const std::string &groupId) {
	ToggleButtonItem *button = new ToggleButtonItem(*this, actionId, getButtonGroup(groupId), myResource[ZLResourceKey(actionId)]);
	myItems.push_back(button);
	return *button;
}

void ZLToolbar::addTextField(const std::string &actionId, const std::string &parameterId, int maxWidth, TextFieldItem::SymbolSet symbolSet) {
	myItems.push_back(new TextFieldItem(*this, actionId, parameterId, maxWidth, symbolSet, myResource[ZLResourceKey(actionId)]));
}

void ZLToolbar::addSeparator() {
	myItems.push_back(new SeparatorItem(*this));
}

ZLToolbar::ButtonGroup::ButtonGroup() : PressedItem(0) {
}

void ZLToolbar::ButtonGroup::press(const ToggleButtonItem *item) {
	PressedItem = item;
}

ZLToolbar::Item::Type ZLToolbar::PlainButtonItem::type() const {
	return PLAIN_BUTTON;
}

ZLToolbar::Item::Type ZLToolbar::MenuButtonItem::type() const {
	return MENU_BUTTON;
}

ZLToolbar::Item::Type ZLToolbar::ToggleButtonItem::type() const {
	return TOGGLE_BUTTON;
}

ZLToolbar::SeparatorItem::SeparatorItem(const ZLToolbar &toolbar) : Item(toolbar) {
}

ZLToolbar::Item::Type ZLToolbar::SeparatorItem::type() const {
	return SEPARATOR;
}

ZLToolbar::TextFieldItem::TextFieldItem(const ZLToolbar &toolbar, const std::string &actionId, const std::string &parameterId, int maxWidth, SymbolSet symbolSet, const ZLResource &tooltip) : ActionItem(toolbar, actionId, tooltip), myParameterId(parameterId), myMaxWidth(maxWidth), mySymbolSet(symbolSet) {
}

const std::string &ZLToolbar::TextFieldItem::parameterId() const {
	return myParameterId;
}

int ZLToolbar::TextFieldItem::maxWidth() const {
	return myMaxWidth;
}

ZLToolbar::TextFieldItem::SymbolSet ZLToolbar::TextFieldItem::symbolSet() const {
	return mySymbolSet;
}

ZLToolbar::Item::Type ZLToolbar::TextFieldItem::type() const {
	return TEXT_FIELD;
}
