if(!$("html").hasClass("js_en")){$("html").addClass("js_en")
}var proxied=((location.hostname.indexOf("onlinelibrary.wiley.com")!=-1)&&(location.hostname.substring(location.hostname.indexOf("onlinelibrary.wiley.com")+23)==""))?false:true;
$(function(){$.ol={};
$(".publicationTypes #allTypes,.subjectsAndAccess #allTopics, #allAsist").searchSelectAll();
$(document).globalMessaging();
if($("#accordion").length>0){$("#accordion").accordion({header:"h2"})
}$(".contextTrigger").contextFilter();
$("#additionalInformation").slider();
$("#login #loggedIn .profile>li").profileMenu();
$("#issueTocGroups, #titles, #publications, .articles, .books").selectAll();
$(".emrw-table").mrwTables();
$(".jumpList").jumpList();
$("#loginEmail, #loginPassword").loginLabels();
if($("#pdf").length){$(window).resize(function(){$("#pdf").children("iframe").height(($(window).height()-$("#pdfHeader").height())-2)
});
$(window).triggerHandler("resize")
}if(!$("#mailUpdates").attr("checked")){$("#mailUpdates").parents("fieldset").next().slideUp()
}$("#mailUpdates").click(function(){if($(this).attr("checked")){$(this).parents("fieldset").next().slideDown()
}else{$(this).parents("fieldset").next().slideUp()
}});
!proxied&&$(".issuesInYear").issueTree();
$("#browseBySubject").subjectTree();
$("#globalMenu ul li:nth-child(3)").addResourceMenu();
$.ol.cleanAJAXResponse=function(d){if(d.indexOf("<body>")>-1){return/<body[^>]*>([\S\s]*?)<\/body>/.exec(d,"ig")[1]
}else{return d
}};
if(!$("#mailPromotionRequested").attr("checked")){$("#mailPromotionRequested").parents("fieldset").next().hide()
}$("#mailPromotionRequested").click(function(){if($(this).attr("checked")){$(this).parents("fieldset").next("fieldset").slideDown()
}else{$(this).parents("fieldset").next("fieldset").slideUp()
}return true
});
$.ol.textSlider=function(e,h,g){var i=0,d=0,f=0;
$(e+" p").each(function(j){i+=$(this).text().length;
if(i>h){if(d==0){f=h-$(this).prev("p").text().length;
d=$(this).text().slice(0,f);
$("<p style='"+$(this).attr("style")+'\'><span id="paragraphFragment">'+d+'&hellip;</span> <a href="#" id="showAboutThisBook">'+g[0]+"</a></p>").appendTo(e);
$("#showAboutThisBook").toggle(function(){$("#paragraphFragment").parent().slideUp(500,function(){$("#showAboutThisBook").css({marginLeft:"0.5em"}).text(g[1]);
$("#allParagraphs").slideDown(500)
});
$("#showAboutThisBook").appendTo("#allParagraphs p:last");
return false
},function(){$("#allParagraphs").slideUp(500,function(){$("#showAboutThisBook").appendTo("#paragraphFragment").text(g[0]).show();
$("#paragraphFragment").parent().slideDown(500)
});
return false
})
}if($("#allParagraphs").length==0){$('<div id="allParagraphs"></div>').appendTo(e)
}$(this).appendTo("#allParagraphs")
}else{$(this).show()
}})
};
$.ol.textSlider("#bookHome #homepageContent",200,["More about this book summary","Less about this book summary"]);
$.ol.textSlider("#bookHomeSeries #homepageContent",200,["More about this book series","Less about this book series"]);
if($("#mrwHome").length>0){$.ol.textSlider("#mrwHome #homepageContent:has(~ #rightColumn, ~ #leftColumn)",200,["More about this book","Less about this book"])
}($.ol.keyStrokeCollector=function(){$(document).keydown(function(f){var d={ctrl:(f.ctrlKey)?true:false,shift:(f.shiftKey)?true:false,alt:(f.altKey)?true:false,key:f.keyCode};
$(this).data("keyDown",d)
});
$(document).keyup(function(d){$(this).removeData("keyDown")
})
})();
$.ol.Tree=function(f,d){this.branches=[];
this.selector=d||f;
var e=this.params={};
$.each($.extend({},$.ol.Tree.defaults,(arguments.length==1)?{}:f),function(h,g){e[h]=g
});
this.init()
};
$.extend($.ol.Tree.prototype,{init:function(){var d=this;
if(!this.params.ajax){$(this.selector).find("li > span").each(function(){var h=$(this).removeAttr("class").wrap('<a href="#"></a>').parents("a").addClass("branchLabel").addClass("open");
d.branches.push(new $.ol.Tree.Branch(h))
});
this.collapseAll()
}else{$(this.selector).find("li > a.branchLabel").each(function(){$(this).click(function(){var i=this,h=$(this).next("ol");
if(h&&h.css("display")=="block"){h.slideUp(function(){$(i).removeClass("open").addClass("closed")
})
}else{if(h&&h.css("display")=="none"){h.slideDown(function(){$(i).removeClass("closed").addClass("open")
})
}else{$.ajax({type:"GET",url:$(i).attr("href"),dataType:"html",dataFilter:function(k,j){return $.ol.cleanAJAXResponse(k)
},beforeSend:function(){$(i).removeClass("closed").addClass("fetching")
},success:function(j){$(i).after($(j).css("display","none"));
$(i).next("ol").slideDown(function(){$(i).removeClass("fetching").addClass("open")
});
$(i).next("ol").mrwTree({ajax:true})
},error:function(j,k,l){$(i).removeClass("fetching").addClass("closed")
}})
}}return false
})
})
}if($(this.selector).find("li > span:first").length){if(this.params.expandAll||this.params.collapseAll){var f=$('<ul class="productMenu"></ul>').prependTo($(this.selector).parent())
}if(this.params.expandAll){var g=$("<li/>").appendTo(f);
$('<a href="#">Expand All</a>').appendTo(g).click(function(){return d.expandAll()
})
}if(this.params.collapseAll){var e=$("<li/>").appendTo(f);
$('<a href="#" id="collapseAll">Collapse All</a>').appendTo(e).click(function(){return d.collapseAll()
})
}}},expandAll:function(){$.each(this.branches,function(){this.expandStep.apply(this.selector,[])
});
return false
},collapseAll:function(){if(this.branches.length!=0){$.each(this.branches,function(){this.collapse.apply(this.selector,[])
})
}else{$(this.selector).find("li > a.open").removeClass("open").addClass("closed").next("ol").slideUp()
}return false
}});
$.ol.Tree.defaults={expandAll:false,collapseAll:false,ajax:false};
$.ol.Tree.Branch=function(d){this.selector=d;
this.init()
};
$.extend($.ol.Tree.Branch.prototype,{init:function(){var d=this;
$(this.selector).click(function(){if($(this).hasClass("closed")){d.expand.apply(this,[])
}else{d.collapse.apply(this,[])
}return false
})
},expand:function(d){$.ol.Tree.Branch.prototype.expandStep.apply(this,[]);
if(d){$(this).parent().find("a:has(span)").each(function(){$.ol.Tree.Branch.prototype.expandStep.apply(this,[])
})
}},expandStep:function(){$(this).removeClass("closed").addClass("open").next("ol").slideDown()
},collapse:function(d){$(this).removeClass("open").addClass("closed").next("ol").slideUp()
}});
$.fn.mrwTree=function(d){return this.each(function(){new $.ol.Tree(d,this)
})
};
!proxied&&$("#mrwBrowseByTopic #browseMrw").mrwTree({ajax:true,collapseAll:true});
$("#mrwBrowseByTOC #browseMrw").mrwTree({expandAll:true,collapseAll:true});
$(".decisionTree").dynamicSelectGroup();
if($("#searchByCitation").length){$("#scope").append('<option value="byCitation">By Citation</option>');
$("#scope").change(function(){if($(this).find("option:selected").text()=="By Citation"){$("#searchByCitation").slideDown(function(){$("#searchText, #searchSiteSubmit").attr("disabled","disabled")
})
}else{$("#searchByCitation").slideUp(function(){$("#searchText, #searchSiteSubmit").removeAttr("disabled")
})
}});
$("#searchByCitation p.error").length&&$("#scope").val("byCitation").triggerHandler("change")
}$("#resourcesMenu").parents("li").hover(function(){$("#resourcesMenu").toggle()
});
$("#resourcesMenu ul").bgiframe({top:20});
$("#issueToc .figZoom img").load(function(){$(this).css("visibility","visible");
if($(this).height()>300){$(this).height(300)
}if($(this).width()>300){$(this).css("float","none")
}}).each(function(){if(this.complete||(jQuery.browser.msie&&parseInt(jQuery.browser.version)==6)){$(this).triggerHandler("load")
}});
$("#fulltext .firstPageContainer img").load(function(){$(this).removeAttr("height").removeAttr("width");
if($(this).width()>752){$(this).width(752)
}}).each(function(){if(this.complete||(jQuery.browser.msie&&parseInt(jQuery.browser.version)==6)){$(this).triggerHandler("load")
}});
$("#fulltext a.movingMolecule").each(function(){$(this).click(function(){window.open($(this).attr("href"),$(this).index(),"width=1024,height=790,scrollbars=yes");
return false
})
});
if($("meta[name='citation_journal_title']").length||($("#pdf").length&&$("#productTitle").length)){var c=($("#pdf").length)?$("#productTitle").attr("href").split("(ISSN)")[1]:$("meta[name='citation_issn']").attr("content");
var a=($("#pdf").length)?window.location.href.split("/doi/")[1].split("/pdf")[0]:$("meta[name='citation_doi']").attr("content");
$.getJSON("http://www.deepdyve.com/rental-link?docId="+a+"&fieldName=journal_doi&journal="+c+"&affiliateId=wiley&format=jsonp&callback=?",function(e){if(e.status==="ok"){var g=$('<p><a class="rentalLink">Rent this article at DeepDyve</a></p>');
var f=$('<div id="deepDyve" class="topLeftRoundCornerNew"><h2>Rent this article through DeepDyve</h2><p>View a read-only copy of this article through our partner DeepDyve, the largest online rental service for scientific and scholarly content. DeepDyve will safeguard user privacy and your information will not be sold to a third party.</p><p>Read more about our pilot program making a portfolio of Biotechnology journals available through DeepDyve.</p><p><a href="http://eu.wiley.com/WileyCDA/PressRelease/pressReleaseId-84017.html">Press Release</a></p></div>');
g.find("a").attr("href",e.url);
f.insertAfter("#accessDenied .access .login");
g.clone(true).insertAfter("#deepDyve p:first");
var d=$('<li><span style="color: red; padding-right: 0.4em; font-weight: bold;">NEW!</span> </li>');
g.find("a").clone(true).appendTo(d);
d.appendTo("#accessDeniedOptions")
}})
}var b=window.institutions||[];
$("#institutionalAndAthensLogin #institutionName").autocomplete({source:b,minLength:2})
});