#pragma once

namespace z0 {

    class GTreeView : public GWidget {
    public:
       
        class Item : public GPanel {
        public:
            shared_ptr<GWidget>     item;
            list<shared_ptr<Item>> 	children;
            bool                    selected{false};
            bool                    expanded{false};

            Item(shared_ptr<GWidget> item);
        };

        GTreeView();

        void setResources(const string&, const string&, const string&);
        void removeAllItems();
        shared_ptr<Item>& addItem(shared_ptr<GWidget> item);
        shared_ptr<Item>& addItem(shared_ptr<Item>& parent, shared_ptr<GWidget> item);
        void expand(shared_ptr<GWidget>& item);

    private:
    	float		            innerHeight;
	    float		            itemsHeight;
	    list<shared_ptr<Item>> 	items;
        shared_ptr<GBox>	    box;
	    shared_ptr<GVScrollBar>	vscroll;

    };

}