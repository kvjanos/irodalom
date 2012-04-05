{
	"translatorID": "c159dcfe-8a53-4301-a499-30f6549c340d",
	"translatorType": 4,
	"label": "DOI",
	"creator": "Simon Kornblith",
	"target": null,
	"minVersion": "1.0.10",
	"maxVersion": null,
	"priority": 300,
	"inRepository": true,
	"browserSupport": "gcs",
	"lastUpdated": "2012-02-22 01:15:00"
}

var items = {};
var selectArray = {};

// builds a list of DOIs
function getDOIs(doc) {
	// TODO Detect DOIs more correctly.
	// The actual rules for DOIs are very lax-- but we're more strict.
	// Specifically, we should allow space characters, and all Unicode
	// characters except for control characters. Here, we're cheating
	// by not allowing ampersands, to fix an issue with getting DOIs
	// out of URLs.
	// Description at: http://www.doi.org/handbook_2000/appendix_1.html#A1-4
	const DOIre = /\b(10\.[\w.]+\/[^\s&]+)\.?\b/igm;
	const DOIXPath = "//text()[contains(., '10.')]";
	
	DOIre.lastMatch = 0;
	var DOIs = [];
	
	var node, m;
	var results = doc.evaluate(DOIXPath, doc, null, XPathResult.ANY_TYPE, null);
	while(node = results.iterateNext()) {
		while(m = DOIre.exec(node.nodeValue)) {
			var DOI = m[1];
			if(DOI.substr(-1) == ")" && DOI.indexOf("(") == -1) {
				DOI = DOI.substr(0, DOI.length-1);
			}
			// only add new DOIs
			if(DOIs.indexOf(DOI) == -1) {
				DOIs.push(DOI);
			}
		}
	}
	
	return DOIs;
}

function detectWeb(doc, url) {
	// Blacklist the advertising iframe in ScienceDirect guest mode:
	// http://www.sciencedirect.com/science/advertisement/options/num/264322/mainCat/general/cat/general/acct/...
	// This can be removed from blacklist when 5c324134c636a3a3e0432f1d2f277a6bc2717c2a hits all clients (Z 3.0+)
	const blacklistRe = /^https?:\/\/[^/]*(?:google\.com|sciencedirect\.com\/science\/advertisement\/)/i;
	
	if(!blacklistRe.test(url)) {
		var DOIs = getDOIs(doc);
		if(DOIs.length) {
			return DOIs.length == 1 ? "journalArticle" : "multiple";
		}
	}
	return false;
}

function retrieveNextDOI(DOIs, doc) {
	if(DOIs.length) {
		// retrieve DOI
		var DOI = DOIs.shift();
		var translate = Zotero.loadTranslator("search");
		translate.setTranslator("11645bd1-0420-45c1-badb-53fb41eeb753");
		var item = {"itemType":"journalArticle", "DOI":DOI};
		translate.setSearch(item);
		// don't save when item is done
		translate.setHandler("itemDone", function(translate, item) {
			item.repository = "CrossRef";
			items[DOI] = item;
			selectArray[DOI] = item.title;
		});
		translate.setHandler("done", function(translate) {
			retrieveNextDOI(DOIs, doc);
		});
		// Don't throw on error
		translate.setHandler("error", function() {});
		translate.translate();
	} else {
		// all DOIs retrieved now
		// check to see if there is more than one DOI
		var numDOIs = 0;
		for(var DOI in selectArray) {
			numDOIs++;
			if(numDOIs == 2) break;
		}
		if(numDOIs == 0) {
			throw "DOI Translator: could not find DOI";
		} else if(numDOIs == 1) {
			// do we want to add URL of the page?
			items[DOI].url = doc.location.href;
			items[DOI].attachments = [{document:doc}];
			items[DOI].complete();
		} else {
			selectArray = Zotero.selectItems(selectArray);
			for(var DOI in selectArray) {
				items[DOI].complete();
			}
		}
		Zotero.done();
	}
}

function doWeb(doc, url) {
	var DOIs = getDOIs(doc);
	// retrieve full items asynchronously
	Zotero.wait();
	retrieveNextDOI(DOIs, doc);
}


/** BEGIN TEST CASES **/
var testCases = [
	{
		"type": "web",
		"url": "http://www.crossref.org/help/Content/01_About_DOIs/What_is_a_DOI.htm",
		"items": [
			{
				"itemType": "journalArticle",
				"creators": [
					{
						"creatorType": "author",
						"firstName": "J",
						"lastName": "Mou"
					}
				],
				"notes": [],
				"tags": [],
				"seeAlso": [],
				"attachments": [
					{
						"document": false
					}
				],
				"publicationTitle": "Journal of Molecular Biology",
				"volume": "248",
				"ISSN": "00222836",
				"issue": "3",
				"date": "1995-5-5",
				"pages": "507-512",
				"DOI": "10.1006/jmbi.1995.0238",
				"url": "http://www.crossref.org/help/Content/01_About_DOIs/What_is_a_DOI.htm",
				"title": "Atomic Force Microscopy of Cholera Toxin B-oligomers Bound to Bilayers of Biologically Relevant Lipids",
				"libraryCatalog": "CrossRef"
			}
		]
	},
	{
		"type": "web",
		"url": "http://blog.apastyle.org/apastyle/digital-object-identifier-doi/",
		"items": "multiple"
	}
]
/** END TEST CASES **/