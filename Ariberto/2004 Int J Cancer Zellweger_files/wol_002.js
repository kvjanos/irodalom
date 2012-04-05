/*
 * WOL jQuery Plugins
 */
(function($) {

    $.fn.searchSelectAll = function(){
        return this.each(function(){
            var root = $(this).closest("ol, ul");
            var that = $(this);
            that.is(":checked") && root.find("li > input").not(that).attr("checked", "checked");
            root.delegate("input", "click", function(){
                if($(this).attr("id") == that.attr("id")){
                    if ($(this).is(":checked")){
                        root.find("li > input").not(that).attr("checked", "checked");
                    } else {
                        root.find("li > input").attr("checked", "");
                    }
                }else{
                    if ($(this).is(":checked")){
                        (root.find("li > input:checked").not(that).length == root.find("li > input").not(that).length) && that.attr("checked", "checked");
                    } else {
                        that.attr("checked", "");
                    }
                }
            });
        });
    };

    $.fn.slider = function(){
        var slideCollection = $("#authorsDetail, #editorsDetail, #publicationHistoryDetails, #howToCite, #errata, #fundingInfo, #isbnInfo, #bookSeriesInfo");
        var slideCount = 0;
        var reveal = $("<a href='#'>(Show All)</a>");
        if (slideCollection.length > 0){
            var toggleSections = $("<p id=\"toggleAddInfo\"></p>");
            slideCollection.each(function(){
                var that = $(this);
                var name;

                that.bind("slider", function(){
                    if ($(this).is(":visible")){
                        $(this).slideUp();
                        slideCount--;
                    } else {
                        $(this).slideDown();
                        slideCount++;
                    }

                    if(slideCount == 0){
                        reveal.text("(Show All)");
                    } else if (slideCount == slideCollection.length){
                        reveal.text("(Hide All)");
                    }
                });

                if($(this).attr("id") == "authorsDetail")
                    name = "Author Information";
                else if($(this).attr("id") == "editorsDetail")
                    name = "Editor Information";
                else if($(this).attr("id") == "publicationHistoryDetails")
                    name = "Publication History";
                else if($(this).attr("id") == "howToCite")
                    name = "How to Cite";
                else if($(this).attr("id") == "fundingInfo")
                    name="Funding Information";
                else if($(this).attr("id") == "isbnInfo")
                    name = "ISBN Information";
                else if($(this).attr("id") == "bookSeriesInfo")
                    name = "Book Series Information";
                else
                    name = "Corrections";

                $("<a href='#'>" + name + "</a>").click(function(){
                    that.trigger("slider");
                    return false;
                }).appendTo(toggleSections);
            });

            toggleSections.insertAfter("#additionalInformation .articleCategory");

            reveal.click(function(){
                if ($(this).text() == "(Show All)") {
                    slideCollection.show();
                    $(this).text("(Hide All)");
                    slideCount = slideCollection.length;
                } else {
                    slideCollection.hide();
                    $(this).text("(Show All)");
                    slideCount = 0;
                }
                return false;
            }).appendTo("#additionalInformation .articleCategory");
        }
    };

    $.fn.subjectTree = function() {

        var hoverAndFocusHandler = function(e) {
            var li = $(this),
            div = li.children("div");

            li.parent().children("li").removeClass("hover");

            div.css("width", li.find("ol").length * 15 +"em");
            li.addClass("hover");

            var heightAndPositionFromTop = div.offset().top + div.outerHeight() + 40;
            if(heightAndPositionFromTop > $("body").height()) {
                var offset = $("body").height() - heightAndPositionFromTop;
                div.css("top", offset);
            }
        };

        return this.each(function() {
            $(this).children("li")
                .hover(hoverAndFocusHandler,function(){
                    $(this).removeClass("hover");
                })
                .children("a")
                .attr("href", "#")
                .keydown(function(e) {
                    if(e.which == 13) {
                        e.preventDefault();
                        var li = $(this).parent();
                        if(li.hasClass("hover")) {
                            li.removeClass("hover");
                        } else {
                            hoverAndFocusHandler.call(li);
                        }
                        return false;
                    }
                })
                .end()
                .find("> div li a")
                .focus(function() {
                    $(this).parent().addClass("hover");
                })
                .blur(function() {
                    $(this).parent().removeClass("hover");
                });
        });
    };

    $.fn.profileMenu = function() {
        return this.each(function() {
            var showing = false;
            $(this).hover(function(){
                if (!showing){
                    $(this).find("ul").fadeIn("fast",function(){});
                    showing=true;
                }
            },
            function(){
                if (showing){
                    $(this).find("ul").fadeOut("slow",function(){showing = false;});
                }
            });
        });
    };

    $.fn.issueTree = function() {
        this.each(function() {
            $(this).click(function() {
                var doiForYear = $(this).attr("href").split("/journal/")[1].split("/issues")[0];
                var year = ($(this).attr("href")).split("?")[1].split("=")[1];
                var branch = $(this).next("ol");

                if(branch && branch.css("display") == "block") {
                    branch.slideUp(function(){
                        $(this).prev().removeClass('open').addClass('closed');
                        storeState();
                    });
                } else if(branch && branch.css("display") == "none") {
                    branch.slideDown(function(){
                        $(this).prev().removeClass('closed').addClass('open');
                        storeState();
                    });
                } else {
                    var that = $(this);
                    $.ajax({
                        type :"GET",
                        url :"/journal/" + doiForYear + "/issues/fragment?activeYear=" + year + "&SKIP_DECORATION=true",
                        dataType : "html",
                        beforeSend : function() {
                            $(that).removeClass('closed').addClass('fetching');
                        },
                        success : function(htmlForIssues) {
                            $(that).after($(htmlForIssues).css("display","none"));
                            $(that).next("ol").slideDown(function() {
                                $(this).prev().removeClass('fetching').addClass('open');
                                storeState();
                            });
                        }
                        ,
                        error : function(x, t, e){
                            $(this).prev().removeClass('fetching').addClass('closed');
                        }
                    });
                }
                return false;
            });
        });
        function storeState() {
            $.cookie("allIssuesTree", null, {path:"/"});
            var data = [];
            $(".issueVolumes>li").each(function(i, e) {
                data[i] = $(e).find("ol").css("display") == "block" ? 1 : 0;
            });
            $.cookie("allIssuesTree", getDoiFromURL()+" "+data.join(""), {path: '/'});
        };
        function restoreState() {
            var state = $.cookie("allIssuesTree");
            if(state && state.substring(0, state.indexOf(" "))==getDoiFromURL() && $(".issueVolumes").length) {
                var data = state.substr(state.indexOf(" ")+1).split("");
                $(".issueVolumes>li").each(function(i, e) {
                    if(data[i] == 1) $(e).find("a").click();
                });
            }
        };
        function getDoiFromURL() {
            var path = window.location.pathname;
            var doi = path.substr(path.indexOf("journal/"));
            doi = doi.substring(0, doi.indexOf("/issues"))
            return doi;
        };
        restoreState();
        return this;
    };

    $.fn.selectAll = function(){
        return this.each(function(n){
            var checkboxes = $(this).find("input[type='checkbox']");
            var selectAlls = $(this).parents("form").find("div.selectAll").find("input[type='checkbox']");
            $(this).parents("form").delegate("input[type='checkbox']", "click", function(){
              if ($(this).parents("div").hasClass("selectAll")){
                if ($(this).is(":checked")){
                  $(checkboxes).attr("checked","checked");
                  $(selectAlls).attr("checked","checked").next("label").text("Deselect All");
                } else {
                  $(checkboxes).removeAttr("checked");
                  $(selectAlls).removeAttr("checked").next("label").text("Select All");
                }
              } else {
                if (checkboxes.filter(":checked").length == checkboxes.length){
                  $(selectAlls).attr("checked","checked").next("label").text("Deselect All");
                } else {
                  $(selectAlls).removeAttr("checked").next("label").text("Select All");
                }
              }
            });
        });
    };

    $.fn.selectAllNew = function(buttonLocs){
      return this.each(function(n){
        var checkbox = $("<div class=\"selectAll\"><input type=\"checkbox\" name=\"selectAll\" class=\"selectAll\" value=\"selectAll\"/><label>Select All</label></div>");
        var selectName = "selectAll"+n;
        checkbox.find("input").attr("id", selectName).next().attr("for", selectName);

        $.each(buttonLocs, function(i,n){
                $(n)[i](checkbox.clone());
        });

        var checkboxes = $(this).find("input[type='checkbox']:not(.selectAll)");
        var selectAlls = $(this).find("input.selectAll");
        $(this).delegate("input[type='checkbox']", "click", function(){
          if ($(this).hasClass("selectAll")){
            if ($(this).is(":checked")){
              $(checkboxes).attr("checked","checked");
              $(selectAlls).attr("checked","checked").next("label").text("Deselect All");
            } else {
              $(checkboxes).removeAttr("checked");
              $(selectAlls).removeAttr("checked").next("label").text("Select All");
            }
          } else {
            if (checkboxes.filter(":checked").length == checkboxes.length){
              $(selectAlls).attr("checked","checked").next("label").text("Deselect All");
            } else {
              $(selectAlls).removeAttr("checked").next("label").text("Select All");
            }
          }
        });
      });
    };

    $.fn.loginLabels = function(){
        return this.each(function(){
            if($(this).val() != ""){
                $(this).prev().hide();
            };
            $(this).focus(function(){
                $(this).prev().hide();
            }).blur(function(){
                if($(this).val() == ""){
                    $(this).prev().show();
                }
            });
        });
    };

    $.fn.contextFilter = function() {
        return $(this).change(function() {
            var that = this;
            $(this).siblings(".contextReceiver").find("optgroup").each(function() {
                $(this).css("display", "block");
                if($(this).attr("label") != $(that).find("option:selected").text()) {
                   $(this).css("display", "none");
                }
            }).parent().eq(0).find("option:first").attr("selected", "selected");
        }).trigger("change");
    };

    $.fn.fileUploadFix = function() {
        return this.each(function() {
            $("label[for='"+$(this).attr("id")+"']")
                .after("<input id=\""+$(this).attr("id")+"Dummy\" class=\"fileUpload\" style=\"width:12.35em\" /><div class=\"dummyFileButton\" id=\""+$(this).attr("id")+"DummySubmit\"><input type=\"button\" value=\"Browse\" class=\"dummyFileButton\" /></div>")
                .add(this)
                .add("#"+$(this).attr("id")+"Dummy")
                .add("#"+$(this).attr("id")+"DummySubmit")
                .wrapAll($("<div class=\"uploadContainer\" />"))
                .eq(1)
                .addClass("fileUpload")
                .css({"opacity":"0","z-index":1})
                .change(function(){
                    $("#"+$(this).attr("id")+"Dummy").val($(this).val());
                }
            ).trigger("change");
        });
    };

    $.fn.addResourceMenu = function(){
      var resources = {
             "1" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-390244.html","text":"Training and Tutorials"},
             "2" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-404512.html","text":"For Researchers"},
             "3" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-404513.html","text":"For Librarians"},
             "4" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-404518.html","text":"For Societies"},
             "5" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-404516.html","text":"For Authors"},
             "6" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-390236.html","text":"For Advertisers"},
             "7" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-390242.html","text":"For Media"},
             "8" : {"link":"http://olabout.wiley.com/WileyCDA/Section/id-390243.html","text":"For Agents"}
            };

        var div = $("<div id=\"resourcesMenu\"></div>");
        var list = $("<ul/>");

        jQuery.each(resources,function(i,v){
            $("<li>"+"<a href="+v['link'] +">"+ v['text'] +"</a></li>").appendTo(list);
        });
        $(list).appendTo(div);
        $(this).append(div);
    };

    $.fn.mrwTables = function() {
        return this.each(function(){
            $("<a href=\"#\" class=\"viewTable\">View table</a>").prependTo($(this).find(".title")).toggle(function() {
                $(this).parent().next(".table-container").slideDown();
                $(this).text("Hide table");
                return false;
            },
            function(){
                $(this).parent().next(".table-container").slideUp();
                $(this).text("View table");
                return false;
            });
        });
    };

    $.fn.jumpList = function(){
        var select = $("<select />")
                        .change(function(){
                            window.location.hash = $(this).val();
                            $(this).val("jumpTo");
                        })
                        .addClass("jumpSelect")
                        .append($("<option />").val("jumpTo").html("Jump to&hellip;"));

        $($.map($(this).eq(0).find("li"), function(el) {
            var link = $(el).find("a:first");

            return $("<option />").val(link.attr("href")).html(link.getAltAndText());
        })).appendTo(select);


        select.css("visibility", "hidden").appendTo("body");
        if(select.width() <= 550) {
            select.css("width", ((select.width() < 238) ? 238 : select.width()+20));
        }else if(select.width() > 500) {
            select.css({"clear":"left", "width":740, "margin-left":0});
        }
        select.detach().css("visibility", "visible");

        return this.each(function(){
            select.clone(true).prependTo($(this).hide().prev());
        });
    };

    $.fn.getAltAndText = function(){
        var linkEl = this.clone();

        if(linkEl.children("img").length) {
            $.each(linkEl.children("img"), function() {
                $(this).replaceWith($(this).attr("alt"));
            });
        }
        return linkEl.html();
    };

})(jQuery);
