#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbox.h"
#include "z0/gui/gtext.h"
#include "z0/gui/gtextedit.h"
#include "z0/application.h"
#endif

namespace z0 {

	GTextEdit::GTextEdit(string TXT): GWidget(TEXTEDIT), 
		selStart(0), selLen(0), startPos(0), nDispChar(0) {
		//transparent = TRUE;
		allowFocus = TRUE;
		text = TXT;
	}

	void GTextEdit::setResources(const string&BRES) {
		add(box, GWidget::FILL, BRES);
		box->add(gtext, GWidget::HCENTER);
		selStart = 0;
		startPos = 0;
		computeNDispChar();
	}

	void GTextEdit::setSelStart(uint32_t S) {
		selStart = S;
		if (startPos > selStart) startPos = selStart;
	}

	void GTextEdit::setText(const string&TEXT) {
		if (text == TEXT) return;
		if (text.empty()) {
			selStart = 0;
			startPos = 0;
		}
		text = TEXT;
		computeNDispChar();
		if (parent) { parent->refresh(); }
		gtext->setText(text.substr(startPos, nDispChar+1));
		box->refresh();
		refresh();
		auto event = GEventTextChange{ .text = text };
        event.source = this;
        emit(GEvent::OnTextChange, &event);
	}

	bool GTextEdit::eventKeybDown(Key K) {
		auto consumed = GWidget::eventKeybDown(K);
		if (isReadOnly()) { return K; }

		setFreezed(true);
		if (K == Key::KEY_LEFT) {
			if (selStart > 0) { selStart--; }
		}
		else if (K == Key::KEY_RIGHT) {
			if (selStart < text.length()) { selStart++; }
		}
		else if (K == Key::KEY_END) {
			selStart = text.length();
		}
		else if (K == Key::KEY_HOME) {
			selStart = 0;
		}
		else if (K == Key::KEY_BACKSPACE) {
			if (selStart > 0) {
				selStart--;
				setText(text.substr(0, selStart) +  text.substr(selStart+1, 
						text.length() - selStart - 1));
			}
		}
		else if (K== Key::KEY_DELETE) {
			if (selStart < text.length()) {
				setText(text.substr(0, selStart) +  text.substr(selStart+1, 
						text.length() - selStart - 1));
			}
		}
		/*else if ((K != keyb->KEY_SHIFTRIGHT) &&
				(K != keyb->KEY_SHIFTLEFT) &&
				(K != keyb->KEY_CTRLRIGHT) &&
				(K != keyb->KEY_CTRLLEFT) &&
				(K != keyb->KEY_ALTRIGHT) &&
				(K != keyb->KEY_ALTLEFT))
		{
			UChar c = keyb->CodeToChar(K);
			if (c >= _WORD(0x0020)) {
				SetText(text.Left(selStart) + c + 
						text.Right(text.Len() - selStart));
				selStart++;
			}
			else {
				Freeze() = FALSE;
				return K;
			}
		}*/
		else {
			setFreezed(false);
			return consumed;
		}
		computeNDispChar();
		if (selStart < startPos) {
			startPos = selStart;
		}
		else if ( ((selStart + selLen) > (startPos + nDispChar)) &&
				(nDispChar != text.length()) ) {
			startPos = selStart - nDispChar;
		}
		computeNDispChar();
		setFreezed(false);
		gtext->setText(text.substr(startPos, nDispChar+1));
		box->refresh();
		refresh();
		return K;
	}

	void GTextEdit::computeNDispChar() {
		die("not implemented");
/* 		uint32_t i;
		auto s = box->getWidth() - box->getHBorder()*2;
		for (i=startPos; (i<text.length()) && (s > 0); i++) {
			if (s < getFont()->getWidth(text[i])) { break; }
			s -= getFont()->getWidth(text[i]);
		}
		nDispChar = i-startPos;
 */	}

	/* bool GTextEdit::haveFocus() {
		GWidget *w = this;
		do	{
			if (w->isFocused()) return TRUE;
			w = w->getParent();
		} while (w);
		return FALSE;
	}
 */

	void GTextEdit::setReadOnly (bool READONLY) {
		readonly = READONLY;
	}

	bool GTextEdit::isReadOnly () const {
		return readonly;
	}

	string GTextEdit::getDisplayedText() const {
		return gtext->getText();
	}


}