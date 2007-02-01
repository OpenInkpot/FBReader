/*
 * Copyright (C) 2007 Nikolay Pultsin <geometer@mawhrin.net>
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

#include <algorithm>

#include "W32Element.h"

W32Box::W32Box() : myHomogeneous(false), myTopMargin(0), myBottomMargin(0), myLeftMargin(0), myRightMargin(0), mySpacing(0) {
}

void W32Box::addElement(W32ElementPtr element) {
	if (!element.isNull()) {
		myElements.push_back(element);
	}
}

void W32Box::allocate(WORD *&p, short &id) const {
	for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
		(*it)->allocate(p, id);
	}
}

int W32Box::allocationSize() const {
	int size = 0;
	for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
		size += (*it)->allocationSize();
	}
	return size;
}

void W32Box::setVisible(bool visible) {
	// TODO: implement
}

int W32Box::controlNumber() const {
	int number = 0;
	for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
		number += (*it)->controlNumber();
	}
	return number;
}

void W32Box::setHomogeneous(bool homogeneous) {
	myHomogeneous = homogeneous;
}

void W32Box::setMargins(int top, int bottom, int left, int right) {
	myTopMargin = top;
	myBottomMargin = bottom;
	myLeftMargin = left;
	myRightMargin = right;
}

void W32Box::setSpacing(int spacing) {
	mySpacing = spacing;
}

void W32Box::setDimensions(Size charDimension) {
	for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
		(*it)->setDimensions(charDimension);
	}
}

W32HBox::W32HBox() {
}

W32Element::Size W32HBox::minimumSize() const {
	if (myElements.empty()) {
		return Size(leftMargin() + rightMargin(), topMargin() + bottomMargin());
	}

	Size size;
	if (homogeneous()) {
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();
			size.Width = std::max(size.Width, elementSize.Width);
			size.Height = std::max(size.Height, elementSize.Height);
		}
		size.Width *= myElements.size();
	} else {
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();
			size.Width += elementSize.Width;
			size.Height = std::max(size.Height, elementSize.Height);
		}
	}
	size.Width += leftMargin() + rightMargin() + spacing() * (myElements.size() - 1);
	size.Height += topMargin() + bottomMargin();
	return size;
}

void W32HBox::setPosition(int x, int y, Size size) {
	if (myElements.empty()) {
		return;
	}

	x += leftMargin();
	y += topMargin();
	if (homogeneous()) {
		const Size elementSize(
			(size.Width - leftMargin() - rightMargin() - spacing() * (myElements.size() - 1)) / myElements.size(),
			size.Height - topMargin() - bottomMargin()
		);
		const short deltaX = elementSize.Width + spacing();
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			(*it)->setPosition(x, y, elementSize);
			x += deltaX;
		}
	} else {
		const int elementHeight = size.Height - topMargin() - bottomMargin();
		Size elementSize;
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();
			elementSize.Height = elementHeight;
			(*it)->setPosition(x, y, elementSize);
			x += elementSize.Width + spacing();
		}
	}
}

W32VBox::W32VBox() {
}

W32Element::Size W32VBox::minimumSize() const {
	if (myElements.empty()) {
		return Size(leftMargin() + rightMargin(), topMargin() + bottomMargin());
	}

	Size size;
	if (homogeneous()) {
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();	
			size.Width = std::max(size.Width, elementSize.Width);
			size.Height = std::max(size.Height, elementSize.Height);
		}
		size.Height *= myElements.size();
	} else {
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();	
			size.Width = std::max(size.Width, elementSize.Width);
			size.Height += elementSize.Height;
		}
	}
	size.Width += leftMargin() + rightMargin();
	size.Height += topMargin() + bottomMargin() + spacing() * (myElements.size() - 1);
	return size;
}

void W32VBox::setPosition(int x, int y, Size size) {
	if (myElements.empty()) {
		return;
	}

	x += leftMargin();
	y += topMargin();
	if (homogeneous()) {
		const Size elementSize(
			size.Width - leftMargin() - rightMargin(),
			(size.Height - topMargin() - bottomMargin() - spacing() * (myElements.size() - 1)) / myElements.size()
		);
		const short deltaY = elementSize.Height + spacing();
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			(*it)->setPosition(x, y, elementSize);
			y += deltaY;
		}
	} else {
		const short elementWidth = size.Width - leftMargin() - rightMargin();
		for (W32ElementList::const_iterator it = myElements.begin(); it != myElements.end(); ++it) {
			Size elementSize = (*it)->minimumSize();
			elementSize.Width = elementWidth;
			(*it)->setPosition(x, y, elementSize);
			y += elementSize.Height + spacing();
		}
	}
}