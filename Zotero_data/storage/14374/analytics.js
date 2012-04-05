Nabble._uacct = "UA-91855-5";
//Nabble.topicDumpSearch = true;
Nabble.notRed = true;

//Nabble.otherAd = "http://www.nuzmo.com/static/ad.js";

Nabble.analytics = function() {
	if( self != top )
		return;
	if( navigator.cookieEnabled && !Nabble.getCookie("v") ) {
		var query = Nabble.getSearchTerms2(document.referrer);
		var visitCounter = "/util/VisitCounter.jtp?referrer=" + encodeURIComponent(document.referrer);
		var hostname;
		if( query ) {
			visitCounter += "&query=" + query;
			hostname = "search_engine";
			Nabble.addSearchEngineChannel();
		} else {
			var a = /:\/\/(.*?)\//.exec(document.referrer);
			hostname = a ? a[1] : 'none';
		}
		if( Nabble.isMailingListForum ) {
			visitCounter += "&ml=y";
		}
		for( var key in Nabble.abTests ) {
			visitCounter += "&abTests=" + key;
			visitCounter += "&" + key + "=" + Nabble.abTests[key];
		}
		Nabble.loadScript(visitCounter);
		Nabble.urchinTracker('/referrer/'+hostname);
	}
	var expires = new Date();
	expires.setTime(expires.getTime()+30*60*1000);
	document.cookie = "v=x; expires=" + expires.toGMTString() + "; path=/";
	var page = Nabble.getPage();
	Nabble.urchinTracker(page);
};

Nabble.getPage = function() {
	switch( Nabble.page ) {

	case 'forum.TopicDump':
		return Nabble.mapPage('topic-dump');
	case 'forum.ViewTopic':
		return Nabble.mapPage('topic-framed');
	case 'forum.ViewPost':
		return Nabble.mapPage('post-page');
	case 'forum.ForumTopics':
		return Nabble.mapPage('forum-page');

	case 'forum.NewTopic':
	case 'forum.Reply':
		Nabble.addHandler( Nabble.get('nabble.save'), 'onclick', 
			function(){Nabble.urchinTracker('/posted');}
		);
		return null;

	case 'user.Login':
		Nabble.urchinTracker('/login/'+Nabble.status);
		return null;

	case 'user.Register':
		Nabble.urchinTracker('/register/'+(Nabble.errorMsg==null?'start':'error/'+Nabble.errorMsg));
		return null;
	case 'user.Register2':
		Nabble.urchinTracker('/register/mailed');
		return null;
	case 'user.Register3':
		if( Nabble.status )
			Nabble.urchinTracker('/register/'+Nabble.status);
		return null;

	case 'forum.Search':
		Nabble.addSearchedChannel();  // for AdSense
		if( location.search.match(/[?&]matchingForums=a/) )
			return '/search/matchingForums'+location.search;
		if( location.search.match(/[?&]local=/) )
			return '/search/1'+location.search;
		return '/search/2'+location.search;

	case 'Index':
		var t = Nabble.get('nabble.new-forum-link')
		if(t) {
			Nabble.addHandler( t, 'onclick', 
				function(){Nabble.urchinTracker('/new_forum_link');}
			);
		}
		return null;

	case 'catalog.EditForum':
		Nabble.addHandler( Nabble.get('nabble.save'), 'onclick', 
			function(){Nabble.urchinTracker('/forum_edit/save');}
		);
		return null;

	default:
		return null;
	}
};

Nabble.mapPage = function(page) {
	return '/mapped/' + page + document.location.pathname;
};

Nabble.analyticsForRedirect = function() {
	switch( Nabble.page ) {

	case 'user.Login':
		Nabble.urchinTracker('/login/done');
		break;

	case 'catalog.NewForum':
		if( !Nabble.isModerator )
			Nabble.urchinTracker('/forum_created');
		break;

	}
};

Nabble.analyticsForTopicDumpSearch = function() {
	var fn = function(){
		Nabble.addSearchedChannel();
		Nabble.urchinTracker('/topic_search_link');
	}
	var a = Nabble.get('nabble.searchResults').getElementsByTagName("a");
	for( var i=0; i<a.length-1; i++ ) {  // last is "more" link
		Nabble.addHandler( a[i], 'onclick', fn );
	}
}


Nabble.urchinTracker = function(page) {
	if( typeof(urchinTracker) != "undefined" ) {
		_uacct = Nabble._uacct;
		urchinTracker(page);
	}
};


// AdSense

/*
Nabble.ads = function() {
	google_color_border = "FFFFFF";
	google_color_bg = "FFFFFF";
	google_color_link = "333333";
	google_color_text = "333333";
	google_color_url = "666666";

	document.write('<div class="ad">');
	if( Nabble.otherAd && Math.random() > 0.5 ) {
		document.write('<script type="text/javascript"');
		document.write('  src="' + Nabble.otherAd + '">');
		document.write('<'+'/script>');
	} else {
		google_ad_client = "pub-6703598369329977";
		google_ad_width = 728;
		google_ad_height = 90;
		google_ad_format = "728x90_as";
		google_ad_type = "text";
		google_ad_channel = Nabble.getChannels();
		document.write('<script type="text/javascript"');
		document.write('  src="http://pagead2.googlesyndication.com/pagead/show_ads.js">');
		document.write('<'+'/script>');
	}
	document.write('</div>');
};
*/

Nabble.getChannels = function() {
	var channels = Nabble.getCookie("channels");
	return channels==null ? "" : channels;
};

Nabble.addChannel = function(channel) {
	var channels = Nabble.getCookie("channels");
	if( channels==null ) {
		channels = channel;
	} else {
		if( channels.indexOf(channel) > -1 )
			return;
		channels += "+" + channel;
	}
	Nabble.setCookie("channels",channels);
};

Nabble.addSearchedChannel = function() {
	Nabble.addChannel("2530262431");
};

Nabble.addSearchEngineChannel = function() {
	Nabble.addChannel("4893802913");
};
