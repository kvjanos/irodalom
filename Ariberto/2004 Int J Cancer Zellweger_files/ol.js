(function($) {

    /****   Objects for binding behaviours only   ****/
    /*
    $.widget('ol.overlayBehaviour',{
        _init:function() {
            this.bind();
            this.destroy();
        },
        bind:function() {
            this.element.click(function() { $(this).overlay();return false; });
        }
    });
    $.widget('ol.imageOverlayBehaviour',$.extend({},$.ol.overlayBehaviour.prototype , {
        bind:function() {
            this.element.click(function() { $(this).imageOverlay();return false; });
        }
    }));
    $.widget('ol.hijaxOverlayBehaviour',$.extend({}, $.ol.overlayBehaviour.prototype, {
        bind:function() {
            this.element.click(function() { $(this).hijaxOverlay();return false; });
        }
    }));
    */
    /****   Widget object instances   ****/
    /*
    $.widget('ol.overlay',{
        _init:function() {
            var self = this;

            if(this.options.modal){
                this.prevOverflow = $("body").css("overflow");
            }

            //inline styles to be classed up
            this.shadow =    $("<div id=\"overlayShadowNew\" style=\"position:absolute;width:100%;height:100%;visibility:hidden;background:#000;opacity:0;-moz-opacity:0;filter:alpha(opacity=-0);\"></div>");
            this.container = $("<div id=\"overlayNew\" style=\"position:absolute;visibility:hidden;background-color:#fff;padding:1em;-moz-border-radius:10px;-webkit-border-radius:10px\"></div>");
            this.close =     $("<div id=\"overlayCloseNew\" style=\"height:30px;width:30px;background-color:black;cursor:pointer\"></div>");

            this.setContent().show();
        },
        setContent:function(content) {
            content = content || this.options.content;

            this.container.html(content);

            //if($(content).get(0).tagName == "IMG") {
                $(content).load(function() {
                    //$.extend(this, this.bounds());
                    return this;
                });
            //} else {
            //    return this;
            //}
        },
        show:function() {
            this.container.appendTo("body");

            var self = this, was = {height:this.container.height(),width:this.container.width()};

            this.container
                .css({height:this.element.find("img").height(),width:this.element.find("img").width()})
                .position({my:"center", at:"center", of:self.element.find("img")})
                .css({"visibility":"visible","z-index":"11"})
                .position({my:"center", at:"center", of:window, offset:"-40 -50", by:function(to) {
                    $(this).animate($.extend(to, was), function() {
                        if(self.options.modal) {
                            self.options.modal && $("body").css({"overflow":"hidden","padding-right":"15px"});
                            self.shadow
                                .appendTo("body")
                                .position({my:"center", at:"center", of:window})
                                .css({"z-index":"10","visibility":"visible"})
                                .animate({"opacity":"0.65","-moz-opacity":"0.65","filter":"alpha(opacity=65)"}, "1000");
                        }
                        self.options.close && self.close.appendTo("body").position({my:"center", at:"center", of:window}).css({"z-index":"20","visibility":"visible"});
                        $(document).trigger("setCaption", self);
                    });
                }
            });

            if(this.options.close) {
                this.close.bind("click.overlay", function() {
                    self.hide();
                });
            }

            if(this.options.hideOnEsc) {
                $(document).bind("keydown.overlay", function(event) {
                    event.keyCode && (event.keyCode == 27) && self.hide();
                });
            }

            $.extend(this, {visible:true});

            //Use aspects to insert this and similar trigger points.
            //Other plugins can then link up to provide composite functionality.
            //this._trigger("show");
        },
        hide:function() {
            $(document).unbind(".overlay");

            $.each([this.shadow, this.container, this.close], function(){this.remove()});

            this.options.modal && $("body").css({"overflow":this.prevOverflow,"padding-right":0});

            $.extend(this, {visible:false});

            this.destroy();
        }
    });

    $.widget('ol.hijaxOverlay',$.extend({},$.ol.overlay.prototype, {
        _init:function() {
            $.ol.overlay.prototype._init.call(this);

            var self = this;
            $(this).bind("ajaxSuccess", function() {
                self.container.find("input[type='submit']").click(function(){
                    self.hide();
                    return true;
                });
            });
        },
        setContent:function() {
            var self = this;
            $.ajax({
                type:self.element.attr("href") ? "GET" : "POST",
                url:this.type == "POST" ? self.element.parents("form").attr("action") : self.element.attr("href"),
                data:this.type == "POST" ? self.element.parents("form").serialize()+"&"+self.element.attr("name")+"="+self.element.attr("value") : "",
                dataType:"html",
                dataFilter:function(data, type) {
                    return $.ol.cleanAJAXResponse(data);
                },
                success:function(data, status) {
                    $.ol.overlay.prototype.setContent.call(self, data);
                },
                error:function(xhr, status, errorMessage) {
                    $.ol.overlay.prototype.setContent.call(self, errorMessage);
                }
            });
            return this;
        }
    }));

    $.widget('ol.imageOverlay',$.extend({},$.ol.overlay.prototype, {
        setContent:function() {
            $.ol.overlay.prototype.setContent.call(this, "<img src=\""+this.element.attr("href")+"\" style=\"width:100%;height:100%\" />");
            return this;
        }
    }));

    $.extend($.ol.overlay, {
        defaults:{
            height:"auto",
            width:"auto",
            modal:false,
            close:true,
            content:"Some default dialog text",
            hideOnEsc:true
        }
    });
    $.extend($.ol.hijaxOverlay, {
        defaults:$.extend({}, $.ol.overlay.defaults, {
            modal:true
        })
    });
    $.extend($.ol.imageOverlay, {
        defaults:$.extend({}, $.ol.overlay.defaults, {
            modal:true
        })
    });

     $.widget('ol.caption',{
        _init:function() {
            var self = this;

            //inline styles to be classed up
            this.container = $("<div id=\"captionNew\" style=\"position:absolute;visibility:hidden;background-color:#fff;padding:1em;-moz-border-radius:10px;-webkit-border-radius:10px\">Some Caption Text</div>");

            $(document).bind('setCaption', function(type, observed) {
                if(self.element.context == observed.element.context) {
                    alert("ob");
                    //self.setContent();
                }
            });
        },
        show:function() {
            this.container.appendTo("body").position({my:"center", at:"center", of:window}).css({"visibility":"visible","z-index":"11"});
        }
    });

    $.extend($.ol.caption, {
        defaults:{
            height:"auto",
            width:"auto"
        }
    });

    $.ol.cleanAJAXResponse = function(data) {
       return /<body[^>]*>([\S\s]*?)<\/body>/.exec(data, "ig")[1];
    }
    */
    //This method creates a widget instance for each item in the jQuery object
    //$("li.citation > a, li.email > a, #viewTermsAndConditions, #viewPrivacyPolicy").hijaxOverlay();
    //$("#fulltext .figZoom").imageOverlay().caption();

    // This method only creates one object instance when needed rather than as many as the jQuery object size
    /*
    $("#fulltext .figZoom").click(function(){
        $(this).imageOverlay();
        return false;
    });
    */

    //Refactored to use a lowPro style behaviour binding layer.
    //This would retain the single object instance while binding unique properties to DOM element inside a closure
    /*
    $("#fulltext .figZoom").imageOverlayBehaviour();
    $("li.citation > a, li.email > a, #viewTermsAndConditions, #viewPrivacyPolicy").hijaxOverlayBehaviour();
    */

$.widget("ol.dynamicSelectGroup", {
    _create:function() {
        this.element.wrap("<fieldset />");
        this.element.parents("fieldset").parent().wrapInner("<form action=\""+this.element.find("a:first").attr("href").split("?")[0]+"\" class=\"simple\" method=\"get\" />");
        this.element.parents("form").unwrap().append("<div class=\"submit\"><input type=\"submit\" value=\"Download\" /></div>");
    },
    _init:function() {
        this._build();
    },
    _build:function(listEl, select) {
        var that = this;

        if(!listEl) {
            this.element.find("~select").remove();
            listEl = this.element;
        } else {
            var listDepth = listEl.parents("ul").length;
            this.element.find("~select").eq(listDepth).find("~select").remove();

            listEl = listEl.find(">li>ul").filter(function(i) {
                return $(this).siblings("span").text() === $(select).find("option:selected").text();
            });
        }

        this.element.parent().children("input[type=hidden]").remove();

        var qSMap = {};
        $.each(listEl.find("a:first").attr("href").split("?")[1].split("&"), function() {
            if(!that.element.parent().children("#"+this.split("=")[0]).length) qSMap[this.split("=")[0]] = this.split("=")[1];
        });

        while(listEl.length) {
            var classAttr = listEl.attr("class"), className = classAttr.indexOf(" ") > 0 ? classAttr.split(" ")[1] : classAttr;
            delete qSMap[className];
            var select = $("<select />")
                .hide()
                .attr({id:className, name:className})
                .bind("change", {"listEl":listEl}, function(event) {
                    that._build(event.data.listEl, this);
                })
                .appendTo(this.element.parent());

            $($.map(listEl.find(">li"), function(el){
                return $("<option />").attr("value", that._getValueFromQS(el, className)).text($(el).children("span, a").text());
            })).appendTo(select.fadeIn("slow"));

            listEl = listEl.find(">li:first>ul:first");
        }

        $.each(qSMap, function(i, v) {
            $("<input />").attr({type:"hidden", id:i, name:i}).val(v).appendTo(that.element.parent());
        });
    },
    _getValueFromQS:function(listItem, className) {
        qs = $(listItem).find("a:first").attr("href").split("?")[1].split("&");

        return $.grep(qs, function(v, i) {
            return v.split("=")[0] === className;
        })[0].split("=")[1];
    },
    destroy:function() {
        this.element.parent().find("select").remove();
        this.element.show();
        $.Widget.prototype.destroy.apply(this, arguments);
    }
});

$.widget("ol.globalMessaging", {
    _create:function() {
        var self = this, opt = self.options;
        self = $.extend(self, {offset:0,showing:false,available:false,timer:0, isIe6:($.browser.msie && $.browser.version == 6)});

        self.messageElement =
            ($("#globalMessaging").length ? $("#globalMessaging") : $("<div/>").attr("id", "globalMessaging").append("<ul/>"))
            .css({top:0,left:0,position:self.isIe6 ? "absolute" : "fixed"})
            .append($("<a id=\"handle\" href=\"#\">"+opt.openedText+"</a>")
                .click(function() {
                    self.showing ? self.hide() : self.show(true);
                    return false;
                })
             )
             .bgIframe();

        // Move to init()?????
        if(self.messageElement.parent().length > 0) {
            self.available = true;
            self.messageElement.detach().prependTo("body");
            self._setOffset();
            self.canShow = ((self.messageElement.find("li").length > 1) || ((self.messageElement.find("li").length == 1) && ((self.messageElement.find("li.maintenanceMessage").length == 0) || (self.messageElement.find("li.maintenanceMessage").length > 0) && ($.cookie("maintenanceMessageViewed") != "true"))));
            !self.canShow && self.hide(true);
        }

        $(window).bind("resize", function() {
            self.available && ((self.showing && self._setOffset()) || (self._setOffset() && self.hide(true)));
        });

        if(self.isIe6) {
            $(window).bind("scroll", function() {
                self.available && ((self.showing && self.messageElement.css({top:$(window).scrollTop()})) || (!self.showing && self.messageElement.css({top:$(window).scrollTop()+self._getOffset()})));
            });
        };

    },
    _init:function() {
        this.show();
    },
    _getOffset:function() {
        return parseInt("-"+this.offset,10);
    },
    _setOffset:function() {
        return this.offset = parseInt(this.messageElement.children("ul").outerHeight(),10);
    },
    addMessage:function(message, className) {
        var self = this;
        self.hide(true);
        self.messageElement.children("ul").append($("<li>"+message+"</li>").attr("class", className));
        self.messageElement.prependTo("#rightBorder");
        self.available = true;
        self._setOffset();
        self.canShow = ((self.messageElement.find("li").length > 1) || ((self.messageElement.find("li").length == 1) && (self.messageElement.find("li.maintenanceMessage").length > 0) && ($.cookie("maintenanceMessageViewed") != "true")));
        if(self.canShow) {
            self.show();
        } else {
            self.hide(true);
        }
    },
    show:function(persist) {
        var self = this, opt = self.options;

        if(this.available && !self.showing && (persist || self.canShow)) {
            self.messageElement.children("ul").css("visibility", "visible");

            self.messageElement
                .animate({left:0,top:self.isIe6 ? $(window).scrollTop() : 0}, opt.animSpeed, function() {
                    self.messageElement.children("a").html(opt.openedText);
                    if(!persist) {
                        self.timer = setTimeout(function() {
                            self.hide();
                        }, opt.delay);
                    }
                    self.showing = true;
                });

             ($.cookie("maintenanceMessageViewed") != "true") && $.cookie("maintenanceMessageViewed", "true", {path:"/"});
        }
    },
    hide:function(reset) {
        var self = this, opt = self.options;

        if(this.available && (self.showing || reset)) {
            clearTimeout(self.timer);
            self.messageElement
                .clearQueue()
                .animate({top:self.isIe6 ? $(window).scrollTop()+self._getOffset() : self._getOffset()}, reset ? 0 : opt.animSpeed, function() {
                    self.messageElement.children("a").html(opt.closedText);
                    //self.messageElement.children("ul").css("visibility", "hidden");
                    self.showing = false;
                 });
        }
    }
});

$.extend($.ol.globalMessaging.prototype, {
    options:{
        animSpeed:"fast",
        delay:5000,
        openedText:"Hide messages",
        closedText:"Show messages"
    }
});

})(jQuery);