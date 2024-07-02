#pragma once

namespace z0 {

    class GTreeView : public GWidget {
    public:
        struct Item {
            shared_ptr<GWidget> item;
            list<Item> 	        children;
            bool                selected{false};
            bool                expanded{false};
        };

        GTreeView();

        void setResources(const string&, const string&, const string&);

    private:
    	float		            innerHeight;
	    float		            itemsHeight;
	    list<Item> 	            items;
        shared_ptr<GBox>	    box;
	    shared_ptr<GVScrollBar>	vscroll;

    };

}