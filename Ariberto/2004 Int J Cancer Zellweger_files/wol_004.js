$(function(){$("#fulltext .figZoom").click(function(){$.fancybox.staticUseSetup();
$.fancybox.showActivity();
var c=$(this);
var b=$(this).attr("href");
var a=new Image();
$(a).load(function(){$.fancybox.hideActivity();
var e=$("<div />").css("padding","10px");
var d=$("<div />");
$(a).appendTo(d);
c.parent().find(".caption").clone().css({"padding-top":"10px"}).appendTo(d);
e.width($(a).attr("width")+20);
d.appendTo(e);
$.fancybox({padding:0,margin:0,content:e,titleShow:false,autoScale:false,onComplete:function(){e.width((e.parent().height()<e.parent().attr("scrollHeight"))?e.width()-20:e.width())
}})
}).attr("src",b);
return false
});
$("#mrwFulltext .fig-thumbnail-href").click(function(){$.fancybox.staticUseSetup();
$.fancybox.showActivity();
var c=$(this);
var b=$(this).attr("href");
var a=new Image();
$(a).load(function(){$.fancybox.hideActivity();
var f=$("<div />").css("padding","10px");
var e=$("<div />");
$(a).appendTo(e);
var d=$("<p />").css({"padding-top":"10px","margin-bottom":"0"});
d.html(c.parent().next().html()).appendTo(e);
f.width($(a).attr("width")+20);
e.appendTo(f);
$.fancybox({padding:0,margin:0,content:f,titleShow:false,autoScale:false,onComplete:function(){f.width((f.parent().height()<f.parent().attr("scrollHeight"))?f.width()-20:f.width())
}})
}).attr("src",b);
return false
});
$(".articleAbstract > .figZoom").fancybox({titleShow:false,cyclic:false,autoScale:false});
$(".referenceLink").click(function(){$.fancybox.staticUseSetup();
var c=$("<ul id='overlayList' />");
var b=$(this).attr("rel");
var a=10;
$.each(b.split("#"),function(d,h){if(h.indexOf("#")==-1&&d!=0){var f=$("#"+h).clone();
var g=f.find("cite");
var e=$("<a />");
e.attr("href","#"+h);
e.click(function(){$.fancybox.close()
});
g.wrap(e);
a+=$("#"+h).height();
f.appendTo(c)
}});
$.fancybox({content:c,width:752,height:a,autoDimensions:false,autoScale:false});
return false
});
$("#viewPrivacyPolicy, #viewTermsAndConditions").fancybox({titleShow:false,width:800,autoDimensions:false,autoScale:false,cyclic:false});
$(".fieldAssistance > a").fancybox({titleShow:false,width:480,height:410,autoDimensions:false,autoScale:false,cyclic:false});
!proxied&&$("li.email > a").fancybox({titleShow:false,width:620,height:410,autoDimensions:false,autoScale:false,cyclic:false});
!proxied&&$("li.citation > a").fancybox({titleShow:false,width:480,height:170,autoDimensions:false,autoScale:false,cyclic:false})
});