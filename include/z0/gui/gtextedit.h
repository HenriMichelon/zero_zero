#pragma once

namespace z0 {

    class GTextEdit: public GWidget {
    public:
        GTextEdit(string = "");
        virtual ~GTextEdit() = default;

        bool isReadOnly() const;
        void setReadOnly(bool);
        
        void setText(const string&);

        void setSelStart(uint32_t);

        const string& getText() const { return (string&)text; }
        uint32_t getSelStart() const { return selStart; }
        uint32_t getFirstDisplayedChar() const { return startPos; };

        // return TRUE if this or parent have keyboard focus
        //bool haveFocus();

        string getDisplayedText() const;

        void setResources(const string&);


    protected:
        string      text;
        bool	    readonly;
        uint32_t	selStart;
        uint32_t	selLen;
        uint32_t	startPos;
        uint32_t	nDispChar;
        shared_ptr<GBox> 	    box;
        shared_ptr<GText>	    gtext;

        bool eventKeybDown(Key) override;

        // Compute the number of displayed characters
        void computeNDispChar();

    };

    

}