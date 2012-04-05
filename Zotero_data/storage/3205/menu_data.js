_menuCloseDelay=150           // The time delay for menus to remain visible on mouse out
_menuOpenDelay=10            // The time delay before menus open on mouse over
_subOffsetTop=-1             // Sub menu top offset
_subOffsetLeft=3            // Sub menu left offset

theURL = location.href;

with(topStyle=new mm_style()){
itemwidth="225px";
//onbgcolor="#FCFCFC";
//oncolor="#0099CC";
overbgimage="http://www.computer.org/portal/cms_docs_cs/csdl/images/overnav_bg.jpg";
offbgcolor="transparent";
offcolor="#002874";
//bordercolor="#CACBCD";
//borderstyle="solid";
//borderwidth="1";
//high3dcolor="#EAEDF3";
//low3dcolor="#000033";
//swap3d="1";
separatorsize="2";
separatorwidth="198";
separatorimage="http://www.computer.org/portal/cms_docs_cs/csdl/images/leftnav_separator.gif";
padding="5";
fontsize="10px";
fontweight="bold";
fontfamily="Verdana, Arial, Helvetica, sans-serif";
pagecolor="#0099CC";
pagebgimage="/portal/cms_docs_cs/csdl/images/onpage_bg.jpg";
//headercolor="#000000";
//headerbgcolor="616181";
//menubgimage="";
//subimage="";
subimagepadding="2";
//overfilter="Fade(duration=0.2);Alpha(opacity=100);Shadow(color='#777777', Direction=135, Strength=5)";
//outfilter="randomdissolve(duration=0.3)";
}

with(menuStyle=new mm_style()){
itemwidth="175px";
onbgcolor="#FFFFFF";
oncolor="#009999";
offbgcolor="#EAEDF3";
offcolor="#000033";
bordercolor="#CACBCD";
borderstyle="solid";
borderwidth="1";
separatorcolor="#CACBCD";
separatorsize="1";
padding="3";
fontsize="11px";
fontstyle="normal";
fontweight="normal";
fontfamily="Verdana, Arial, Helvetica, sans-serif";
pagecolor="#000033";
pagebgcolor="#FFFFFF";
//headercolor="#000000";
//headerbgcolor="#616181";
//subimage="";
//subimagepadding="2";
overfilter="Fade(duration=0.2);Alpha(opacity=100);Shadow(color='#777777', Direction=135, Strength=5)";
//outfilter="randomdissolve(duration=0.3)";
}


with(milonic=new menuname("dlib Left Nav")){
style=topStyle;
itemwidth="200";
top=130;
left=2;
alwaysvisible=1;
//orientation="horizontal";
	
	aI("text=DIGITAL LIBRARY HOME;url=http://www.computer.org/portal/pages/csdl/content/index.html;pagematch=csdl/content/&file=index.xm;");
	aI("text=BROWSE BY TITLE;url=http://www.computer.org/portal/pages/csdl/content/index.html;");
	aI("text=BROWSE BY SUBJECT;url=http://www.computer.org/portal/pages/csdl/content/subject.html;pagematch=csdl/content/&file=subject.xm");
	aI("text=SEARCH;showmenu=search; target=_blank;");
	aI("text=LIBRARY/INSTITUTION RESOURCES;showmenu=instresources;pagematch=csdl/library/&file=resource.xml;pagematch=csdl/library/&file=terms.xml;");
	aI("text=RESOURCES;showmenu=resources;pagematch=csdl/jsp/rss/index.jsp;");
	aI("text=SUBSCRIPTION;showmenu=subscribe;pagematch=csdl/library/&file=LibrarySubscription.xml;");
	aI("text=ABOUT THE DIGITAL LIBRARY;url=http://www.computer.org/portal/pages/csdl/content/about.html;pagematch=csdl/content/&file=about.xml");
	aI("img=http://www.computer.org/portal/cms_docs_cs/csdl/images/leftnav_separator.gif;");
}

with(milonic=new menuname("search")){
style=menuStyle;
	aI("text=Basic Search;url=http://search3.computer.org/search/simpleSearch.jsp; target=_blank;");
	aI("text=Advanced Search;url=http://search3.computer.org/search/advancedSearch.jsp; target=_blank;");
	aI("text=Search by Author;url=http://search3.computer.org/search/authorSearch.jsp; target=_blank;");
	aI("text=Search Conference Proceedings;url=http://search3.computer.org/search/proceedingsSearch.jsp; target=_blank;");
}

with(milonic=new menuname("instresources")){
style=menuStyle;
	aI("text=OPAC;url=http://www.computer.org/portal/pages/csdl/library/resource.html; target=_blank;pagematch=csdl/library/&file=resource.xml;");
	aI("text=Library/Institution Newsletter;url=http://www.computer.org/portal/cms_docs_cs/csdl/jsp/sendmail/cslsp/index.jsp?content=yes;");
	aI("text=Library/Institution Subscriptions;url=http://www.computer.org/portal/pages/csdl/library/LibrarySubscription.html;pagematch=csdl/library/&file=LibrarySubscription.xml;");
}

with(milonic=new menuname("resources")){
style=menuStyle;
	aI("text=RSS Feeds;url=http://www.computer.org/portal/cms_docs_cs/csdl/jsp/rss/index.jsp?content=yes;pagematch=csdl/jsp/rss/index.jsp;");
	aI("text=CSDL Newsletter;url=http://www.computer.org/portal/cms_docs_cs/csdl/jsp/sendmail/csdl/csdl_subscribe.jsp;");
	aI("text=Terms of Use;url=http://www.computer.org/portal/pages/csdl/content/Terms.html;pagematch=csdl/content/&file=terms.xml");
	aI("text=Peer Review Notice;url=http://www.computer.org/portal/pages/csdl/content/peer_review.html;pagematch=csdl/content/&file=peer_review.xml");
}

with(milonic=new menuname("subscribe")){
style=menuStyle;
	aI("text=Member Subscriptions;url=http://www.computer.org/portal/pages/ieeecs/publications/subscribe/index.html; target=_blank;");
	aI("text=Library/Institution Subscriptions;url=http://www.computer.org/portal/pages/csdl/library/LibrarySubscription.html;pagematch=csdl/library/&file=LibrarySubscription.xml;");
}

drawMenus();
