/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "HTMLObjectElement.h"

#include "Attribute.h"
#include "CSSValueKeywords.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "FormDataList.h"
#include "Frame.h"
#include "HTMLDocument.h"
#include "HTMLFormElement.h"
#include "HTMLImageLoader.h"
#include "HTMLMetaElement.h"
#include "HTMLNames.h"
#include "HTMLParamElement.h"
#include "HTMLParserIdioms.h"
#include "MIMETypeRegistry.h"
#include "NodeList.h"
#include "Page.h"
#include "PluginViewBase.h"
#include "RenderEmbeddedObject.h"
#include "RenderImage.h"
#include "RenderWidget.h"
#include "ScriptEventListener.h"
#include "Settings.h"
#include "Text.h"
#include "Widget.h"
#include "FrameView.h"

namespace WebCore {

using namespace HTMLNames;

inline HTMLObjectElement::HTMLObjectElement(const QualifiedName& tagName, Document* document, HTMLFormElement* form, bool createdByParser)
    : HTMLPlugInImageElement(tagName, document, createdByParser, ShouldNotPreferPlugInsForImages)
    , FormAssociatedElement(form)
    , m_docNamedItem(true)
    , m_useFallbackContent(false)
    , m_widget( 0 )
    , m_oipfType( NO_OIPF_OBJ )
{

#ifdef WEBCORE_DEBUG
	fprintf( stderr, "HTMLObject Constructor this %x\n", this );
#endif

    ASSERT(hasTagName(objectTag));
    if (!this->form())
        setForm(findFormAncestor());
    if (this->form())
        this->form()->registerFormElement(this);
}

inline HTMLObjectElement::~HTMLObjectElement()
{
    if (form())
        form()->removeFormElement(this);

	fprintf( stderr, "HTMLObject Destructor %x\n", ( m_widget == 0 ) );
	if( m_widget )
	{
		if( document() && document()->view() )
		{
			document()->view()->removeChild( m_widget.get() );
		}
		else
		{
			m_widget->setParent( NULL );
		}

		m_widget = 0;
	}
}

PassRefPtr<HTMLObjectElement> HTMLObjectElement::create(const QualifiedName& tagName, Document* document, HTMLFormElement* form, bool createdByParser)
{
    return adoptRef(new HTMLObjectElement(tagName, document, form, createdByParser));
}

RenderWidget* HTMLObjectElement::renderWidgetForJSBindings()
{
    document()->updateLayoutIgnorePendingStylesheets();
    return renderPart(); // This will return 0 if the renderer is not a RenderPart.
}

void HTMLObjectElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == formAttr)
        formAttributeChanged();
    else if (attr->name() == typeAttr) {
        m_serviceType = attr->value().lower();
        size_t pos = m_serviceType.find(";");
        if (pos != notFound)
            m_serviceType = m_serviceType.left(pos);
        if (renderer())
            setNeedsWidgetUpdate(true);
        if (!isImageType() && m_imageLoader)
            m_imageLoader.clear();

		/* oipfType */

		if (equalIgnoringCase( m_serviceType, "application/oipfobjectfactory") )
		{
			m_oipfType = OIPF_OBJ_FACTORY;
		}
		else if( equalIgnoringCase(serviceType(), "application/oipfapplicationmanager" ) )
		{
			m_oipfType = OIPF_APP_MANANGER;
		}
		else if( equalIgnoringCase(serviceType(), "application/oipfconfiguration"))
		{
			m_oipfType = OIPF_CONFIGURATION;
		}
		else if( equalIgnoringCase(serviceType(), "application/oipfcapabilities"))
		{
			m_oipfType = OIPF_CAPABILITIES;
		}
		else if( equalIgnoringCase(serviceType(), "application/oipfdrmagent"))
		{
			m_oipfType = OIPF_DRMAGENT;
		}
		else if( equalIgnoringCase(serviceType(), "video/broadcast") )
		{
			m_oipfType = OIPF_VIDEO_BC;
		}
		else if( equalIgnoringCase(serviceType(), "valups/system") )
		{
			m_oipfType = VALUPS_SYSTEM;
		}
		else if( equalIgnoringCase(serviceType(), "humax/portalprofile") )
		{
			m_oipfType = H_PORTAL_PROFILE;
		}
		else if( equalIgnoringCase(serviceType(), "video/mp4")
			|| equalIgnoringCase(serviceType(), "video/mpeg")
			|| equalIgnoringCase(serviceType(), "video/mpeg4")
			|| equalIgnoringCase(serviceType(), "application/x-mpegURL"))
		{
			m_oipfType = OIPF_VIDEO_MPEG;
		}
    } else if (attr->name() == dataAttr) {
        m_url = stripLeadingAndTrailingHTMLSpaces(attr->value());
        if (renderer()) {
            setNeedsWidgetUpdate(true);
            if (isImageType()) {
                if (!m_imageLoader)
                    m_imageLoader = adoptPtr(new HTMLImageLoader(this));
                m_imageLoader->updateFromElementIgnoringPreviousError();
            }
        }
    } else if (attr->name() == classidAttr) {
        m_classId = attr->value();
        if (renderer())
            setNeedsWidgetUpdate(true);
    } else if (attr->name() == onloadAttr)
        setAttributeEventListener(eventNames().loadEvent, createAttributeEventListener(this, attr));
    else if (attr->name() == onbeforeloadAttr)
        setAttributeEventListener(eventNames().beforeloadEvent, createAttributeEventListener(this, attr));
    else if (attr->name() == nameAttr) {
        const AtomicString& newName = attr->value();
        if (isDocNamedItem() && inDocument() && document()->isHTMLDocument()) {
            HTMLDocument* document = static_cast<HTMLDocument*>(this->document());
            document->removeNamedItem(m_name);
            document->addNamedItem(newName);
        }
        m_name = newName;
    } else if (attr->name() == borderAttr)
        applyBorderAttribute(attr);
    else if (isIdAttributeName(attr->name())) {
        const AtomicString& newId = attr->value();
        if (isDocNamedItem() && inDocument() && document()->isHTMLDocument()) {
            HTMLDocument* document = static_cast<HTMLDocument*>(this->document());
            document->removeExtraNamedItem(m_id);
            document->addExtraNamedItem(newId);
        }
        m_id = newId;
        // also call superclass
        HTMLPlugInImageElement::parseMappedAttribute(attr);
    } else
        HTMLPlugInImageElement::parseMappedAttribute(attr);
}

static void mapDataParamToSrc(Vector<String>* paramNames, Vector<String>* paramValues)
{
    // Some plugins don't understand the "data" attribute of the OBJECT tag (i.e. Real and WMP
    // require "src" attribute).
    int srcIndex = -1, dataIndex = -1;
    for (unsigned int i = 0; i < paramNames->size(); ++i) {
        if (equalIgnoringCase((*paramNames)[i], "src"))
            srcIndex = i;
        else if (equalIgnoringCase((*paramNames)[i], "data"))
            dataIndex = i;
    }

    if (srcIndex == -1 && dataIndex != -1) {
        paramNames->append("src");
        paramValues->append((*paramValues)[dataIndex]);
    }
}

// FIXME: This function should not deal with url or serviceType!
void HTMLObjectElement::parametersForPlugin(Vector<String>& paramNames, Vector<String>& paramValues, String& url, String& serviceType)
{
    HashSet<StringImpl*, CaseFoldingHash> uniqueParamNames;
    String urlParameter;

    // Scan the PARAM children and store their name/value pairs.
    // Get the URL and type from the params if we don't already have them.
    for (Node* child = firstChild(); child; child = child->nextSibling()) {
        if (!child->hasTagName(paramTag))
            continue;

        HTMLParamElement* p = static_cast<HTMLParamElement*>(child);
        String name = p->name();
        if (name.isEmpty())
            continue;

        uniqueParamNames.add(name.impl());
        paramNames.append(p->name());
        paramValues.append(p->value());

        // FIXME: url adjustment does not belong in this function.
        if (url.isEmpty() && urlParameter.isEmpty() && (equalIgnoringCase(name, "src") ||
			equalIgnoringCase(name, "movie") || equalIgnoringCase(name, "code") || equalIgnoringCase(name, "url")|| equalIgnoringCase(name, "FileName")))
            urlParameter = stripLeadingAndTrailingHTMLSpaces(p->value());
        // FIXME: serviceType calculation does not belong in this function.
        if (serviceType.isEmpty() && equalIgnoringCase(name, "type")) {
            serviceType = p->value();
            size_t pos = serviceType.find(";");
            if (pos != notFound)
                serviceType = serviceType.left(pos);
        }
    }

    // When OBJECT is used for an applet via Sun's Java plugin, the CODEBASE attribute in the tag
    // points to the Java plugin itself (an ActiveX component) while the actual applet CODEBASE is
    // in a PARAM tag. See <http://java.sun.com/products/plugin/1.2/docs/tags.html>. This means
    // we have to explicitly suppress the tag's CODEBASE attribute if there is none in a PARAM,
    // else our Java plugin will misinterpret it. [4004531]
    String codebase;
    if (MIMETypeRegistry::isJavaAppletMIMEType(serviceType)) {
        codebase = "codebase";
        uniqueParamNames.add(codebase.impl()); // pretend we found it in a PARAM already
    }

    // Turn the attributes of the <object> element into arrays, but don't override <param> values.
    NamedNodeMap* attributes = this->attributes(true);
    if (attributes) {
        for (unsigned i = 0; i < attributes->length(); ++i) {
            Attribute* it = attributes->attributeItem(i);
            const AtomicString& name = it->name().localName();
            if (!uniqueParamNames.contains(name.impl())) {
                paramNames.append(name.string());
                paramValues.append(it->value().string());
            }
        }
    }

    mapDataParamToSrc(&paramNames, &paramValues);

    // HTML5 says that an object resource's URL is specified by the object's data
    // attribute, not by a param element. However, for compatibility, allow the
    // resource's URL to be given by a param named "src", "movie", "code" or "url"
    // if we know that resource points to a plug-in.
    if (url.isEmpty() && !urlParameter.isEmpty()) {
        SubframeLoader* loader = document()->frame()->loader()->subframeLoader();
        if (loader->resourceWillUsePlugin(urlParameter, serviceType, shouldPreferPlugInsForImages()))
            url = urlParameter;
    }
}


bool HTMLObjectElement::hasFallbackContent() const
{
    for (Node* child = firstChild(); child; child = child->nextSibling()) {
        // Ignore whitespace-only text, and <param> tags, any other content is fallback content.
        if (child->isTextNode()) {
            if (!static_cast<Text*>(child)->containsOnlyWhitespace())
                return true;
        } else if (!child->hasTagName(paramTag))
            return true;
    }
    return false;
}

bool HTMLObjectElement::shouldAllowQuickTimeClassIdQuirk()
{
    // This site-specific hack maintains compatibility with Mac OS X Wiki Server,
    // which embeds QuickTime movies using an object tag containing QuickTime's
    // ActiveX classid. Treat this classid as valid only if OS X Server's unique
    // 'generator' meta tag is present. Only apply this quirk if there is no
    // fallback content, which ensures the quirk will disable itself if Wiki
    // Server is updated to generate an alternate embed tag as fallback content.
    if (!document()->page()
        || !document()->page()->settings()->needsSiteSpecificQuirks()
        || hasFallbackContent()
        || !equalIgnoringCase(classId(), "clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B"))
        return false;

    RefPtr<NodeList> metaElements = document()->getElementsByTagName(HTMLNames::metaTag.localName());
    unsigned length = metaElements->length();
    for (unsigned i = 0; i < length; ++i) {
        ASSERT(metaElements->item(i)->isHTMLElement());
        HTMLMetaElement* metaElement = static_cast<HTMLMetaElement*>(metaElements->item(i));
        if (equalIgnoringCase(metaElement->name(), "generator") && metaElement->content().startsWith("Mac OS X Server Web Services Server", false))
            return true;
    }

    return false;
}

bool HTMLObjectElement::hasValidClassId()
{
#if PLATFORM(QT)
    if (equalIgnoringCase(serviceType(), "application/x-qt-plugin") || equalIgnoringCase(serviceType(), "application/x-qt-styled-widget"))
        return true;
#endif

    if (MIMETypeRegistry::isJavaAppletMIMEType(serviceType()) && classId().startsWith("java:", false))
        return true;

    if (shouldAllowQuickTimeClassIdQuirk())
        return true;

    if (equalIgnoringCase(serviceType(), "application/x-oleobject")&&
		equalIgnoringCase( classId(), "CLSID:22d6f312-b0f6-11d0-94ab-0080c74c7e95" ))
    {
        return true;
    }



    // HTML5 says that fallback content should be rendered if a non-empty
    // classid is specified for which the UA can't find a suitable plug-in.
    return classId().isEmpty();
}

Widget* HTMLObjectElement::pluginWidget()
{
	if( m_widget && m_widget.get() )
	{
		return m_widget.get();
	}

    return HTMLPlugInImageElement::pluginWidget();
}


void HTMLObjectElement::setWidget(PassRefPtr<Widget> widget)
{
	if( m_oipfType == NO_OIPF_OBJ )
	{
		fprintf( stderr, "!!!!!! NO_OIPF_OBJ!!!!!!!\n" );
		return;
	}

    if ( widget == m_widget )
        return;

	m_widget = widget;
}

#define OIPF_HAS_NO_DISPLAY( a ) (( a == OIPF_APP_MANANGER ) ||\
									( a == OIPF_CONFIGURATION ) || \
									( a == OIPF_CAPABILITIES ) || \
									( a == OIPF_DRMAGENT ) || \
									( a == OIPF_OBJ_FACTORY ) || \
									( a == VALUPS_SYSTEM ) || \
									( a == H_PORTAL_PROFILE ) )

void HTMLObjectElement::updateWidgetIfNecessary()
{
    document()->updateStyleIfNeeded();

//	fprintf( stderr, "serviceType = %s\n", m_serviceType.ascii().data() );

    if ((!needsWidgetUpdate() || useFallbackContent() || isImageType()) &&
		( m_oipfType == NO_OIPF_OBJ ) )
    {
        return;
    }

	if( m_oipfType != NO_OIPF_OBJ ) //( renderEmbeddedObject() == NULL && m_oipfType != NO_OIPF_OBJ ) || ( OIPF_HAS_NO_DISPLAY( m_oipfType ) ) )
	{
		if( m_widget == 0 )
		{
			setNeedsWidgetUpdate(false);
			// FIXME: This should ASSERT isFinishedParsingChildren() instead.
			if (!isFinishedParsingChildren() && !OIPF_HAS_NO_DISPLAY( m_oipfType ) )
			{
				return;
			}

			String url = this->url();
			String serviceType = this->serviceType();

			// FIXME: These should be joined into a PluginParameters class.
			Vector<String> paramNames;
			Vector<String> paramValues;
			parametersForPlugin(paramNames, paramValues, url, serviceType);

			// Note: url is modified above by parametersForPlugin.
			if (!allowedToLoadFrameURL(url))
			{
				return;
			}

			ASSERT(!m_inBeforeLoadEventHandler);
			m_inBeforeLoadEventHandler = true;
			bool beforeLoadAllowedLoad = dispatchBeforeLoadEvent(url);
			m_inBeforeLoadEventHandler = false;

			// beforeload events can modify the DOM, potentially causing
			// RenderWidget::destroy() to be called.  Ensure we haven't been
			// destroyed before continuing.
			// FIXME: Should this render fallback content?
	//		if (!renderer())
		//		return;

			RefPtr<HTMLObjectElement> protect(this); // Loading the plugin might remove us from the document.
			SubframeLoader* loader = document()->frame()->loader()->subframeLoader();

			if( beforeLoadAllowedLoad && hasValidClassId() )
				loader->requestObjectWithoutRenderer(this, url, getAttribute(nameAttr), serviceType, paramNames, paramValues);

			if( m_widget )
			{
				if( document() && document()->view() )
				{
					document()->view()->addChild( m_widget );
				}
			}
		}

		return;
	}

    if (!needsWidgetUpdate() || useFallbackContent() || isImageType())
    {
        return;
    }

	return HTMLPlugInImageElement::updateWidgetIfNecessary();
 }


// FIXME: This should be unified with HTMLEmbedElement::updateWidget and
// moved down into HTMLPluginImageElement.cpp
void HTMLObjectElement::updateWidget(PluginCreationOption pluginCreationOption)
{
//	if(( m_oipfType != NO_OIPF_OBJ ))// OIPF_HAS_NO_DISPLAY( m_oipfType ) )
//	{
//		fprintf( stderr, "ERROR>>>>>>>>>>>>>>>>> oipfType = %d\n", m_oipfType );
//		return;
//	}

    ASSERT(!renderEmbeddedObject()->pluginCrashedOrWasMissing());
    // FIXME: We should ASSERT(needsWidgetUpdate()), but currently
    // FrameView::updateWidget() calls updateWidget(false) without checking if
    // the widget actually needs updating!
    setNeedsWidgetUpdate(false);
    // FIXME: This should ASSERT isFinishedParsingChildren() instead.
    if (!isFinishedParsingChildren())
        return;

    String url = this->url();
    String serviceType = this->serviceType();

    // FIXME: These should be joined into a PluginParameters class.
    Vector<String> paramNames;
    Vector<String> paramValues;
    parametersForPlugin(paramNames, paramValues, url, serviceType);

    // Note: url is modified above by parametersForPlugin.
    if (!allowedToLoadFrameURL(url))
        return;

    bool fallbackContent = hasFallbackContent();
    renderEmbeddedObject()->setHasFallbackContent(fallbackContent);

    if (pluginCreationOption == CreateOnlyNonNetscapePlugins && wouldLoadAsNetscapePlugin(url, serviceType))
        return;

    ASSERT(!m_inBeforeLoadEventHandler);
    m_inBeforeLoadEventHandler = true;
    bool beforeLoadAllowedLoad = dispatchBeforeLoadEvent(url);
    m_inBeforeLoadEventHandler = false;

    // beforeload events can modify the DOM, potentially causing
    // RenderWidget::destroy() to be called.  Ensure we haven't been
    // destroyed before continuing.
    // FIXME: Should this render fallback content?
    if (!renderer())
        return;

    RefPtr<HTMLObjectElement> protect(this); // Loading the plugin might remove us from the document.
    SubframeLoader* loader = document()->frame()->loader()->subframeLoader();
    bool success = beforeLoadAllowedLoad && hasValidClassId() && loader->requestObject(this, url, getAttribute(nameAttr), serviceType, paramNames, paramValues);

    if (!success && fallbackContent)
        renderFallbackContent();
}

bool HTMLObjectElement::rendererIsNeeded(const NodeRenderingContext& context)
{
    // FIXME: This check should not be needed, detached documents never render!
    Frame* frame = document()->frame();
    if (!frame)
        return false;

	/* hbbtv Object for OIPF will be always rendered
    if (equalIgnoringCase(serviceType(), "application/oipfobjectfactory")
		|| equalIgnoringCase(serviceType(), "application/oipfapplicationmanager")
		|| equalIgnoringCase(serviceType(), "application/oipfconfiguration")
		|| equalIgnoringCase(serviceType(), "video/broadcast")
		|| equalIgnoringCase(serviceType(), "video/mp4")
		|| equalIgnoringCase(serviceType(), "video/mpeg")
		|| equalIgnoringCase(serviceType(), "video/mpeg4")
		)
    {
		fprintf( stderr, "%s %s %d\n", __FILE__, __func__, __LINE__ );
		return true;
    }
	/* hbbtv Object for OIPF will be always rendered */

//	if( m_oipfType != NO_OIPF_OBJ &&
//		m_widget )
//	{
//		fprintf( stderr, "Do not render - m_widget exists\n" );
//		return false;
//	}

    return HTMLPlugInImageElement::rendererIsNeeded(context);
}

PassRefPtr<Widget> HTMLObjectElement::getPredefinedWidget()
{
	return m_widget;
}

#if ENABLE(NETSCAPE_PLUGIN_API)

NPObject* HTMLObjectElement::getNPObject()
{
	if( !document() || !document()->frame() )
		return NULL;

	return HTMLPlugInElement::getNPObject();
}

#endif /* ENABLE(NETSCAPE_PLUGIN_API) */


void HTMLObjectElement::insertedIntoDocument()
{
    HTMLPlugInImageElement::insertedIntoDocument();
    if (!inDocument())
        return;

    if (isDocNamedItem() && document()->isHTMLDocument()) {
        HTMLDocument* document = static_cast<HTMLDocument*>(this->document());
        document->addNamedItem(m_name);
        document->addExtraNamedItem(m_id);
    }

    FormAssociatedElement::insertedIntoDocument();
}

void HTMLObjectElement::removedFromDocument()
{
    if (isDocNamedItem() && document()->isHTMLDocument()) {
        HTMLDocument* document = static_cast<HTMLDocument*>(this->document());
        document->removeNamedItem(m_name);
        document->removeExtraNamedItem(m_id);
    }

    HTMLPlugInImageElement::removedFromDocument();
    FormAssociatedElement::removedFromDocument();
}

void HTMLObjectElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    updateDocNamedItem();
    if (inDocument() && !useFallbackContent()) {
        setNeedsWidgetUpdate(true);
        setNeedsStyleRecalc();
    }
    HTMLPlugInImageElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);
}

bool HTMLObjectElement::isURLAttribute(Attribute *attr) const
{
    return (attr->name() == dataAttr || (attr->name() == usemapAttr && attr->value().string()[0] != '#'));
}

const QualifiedName& HTMLObjectElement::imageSourceAttributeName() const
{
    return dataAttr;
}

void HTMLObjectElement::renderFallbackContent()
{
    if (useFallbackContent())
        return;

    if (!inDocument())
        return;

    // Before we give up and use fallback content, check to see if this is a MIME type issue.
    if (m_imageLoader && m_imageLoader->image() && m_imageLoader->image()->status() != CachedResource::LoadError) {
        m_serviceType = m_imageLoader->image()->response().mimeType();
        if (!isImageType()) {
            // If we don't think we have an image type anymore, then clear the image from the loader.
            m_imageLoader->setImage(0);
            reattach();
            return;
        }
    }

    m_useFallbackContent = true;

    // FIXME: Style gets recalculated which is suboptimal.
    detach();
    attach();
}

// FIXME: This should be removed, all callers are almost certainly wrong.
static bool isRecognizedTagName(const QualifiedName& tagName)
{
    DEFINE_STATIC_LOCAL(HashSet<AtomicStringImpl*>, tagList, ());
    if (tagList.isEmpty()) {
        size_t tagCount = 0;
        QualifiedName** tags = HTMLNames::getHTMLTags(&tagCount);
        for (size_t i = 0; i < tagCount; i++) {
            if (*tags[i] == bgsoundTag
                || *tags[i] == commandTag
                || *tags[i] == detailsTag
                || *tags[i] == figcaptionTag
                || *tags[i] == figureTag
                || *tags[i] == summaryTag
                || *tags[i] == trackTag) {
                // Even though we have atoms for these tags, we don't want to
                // treat them as "recognized tags" for the purpose of parsing
                // because that changes how we parse documents.
                continue;
            }
            tagList.add(tags[i]->localName().impl());
        }
    }
    return tagList.contains(tagName.localName().impl());
}

void HTMLObjectElement::updateDocNamedItem()
{
    // The rule is "<object> elements with no children other than
    // <param> elements, unknown elements and whitespace can be
    // found by name in a document, and other <object> elements cannot."
    bool wasNamedItem = m_docNamedItem;
    bool isNamedItem = true;
    Node* child = firstChild();
    while (child && isNamedItem) {
        if (child->isElementNode()) {
            Element* element = static_cast<Element*>(child);
            // FIXME: Use of isRecognizedTagName is almost certainly wrong here.
            if (isRecognizedTagName(element->tagQName()) && !element->hasTagName(paramTag))
                isNamedItem = false;
        } else if (child->isTextNode()) {
            if (!static_cast<Text*>(child)->containsOnlyWhitespace())
                isNamedItem = false;
        } else
            isNamedItem = false;
        child = child->nextSibling();
    }
    if (isNamedItem != wasNamedItem && document()->isHTMLDocument()) {
        HTMLDocument* document = static_cast<HTMLDocument*>(this->document());
        if (isNamedItem) {
            document->addNamedItem(m_name);
            document->addExtraNamedItem(m_id);
        } else {
            document->removeNamedItem(m_name);
            document->removeExtraNamedItem(m_id);
        }
    }
    m_docNamedItem = isNamedItem;
}

bool HTMLObjectElement::containsJavaApplet() const
{
    if (MIMETypeRegistry::isJavaAppletMIMEType(getAttribute(typeAttr)))
        return true;

    for (Element* child = firstElementChild(); child; child = child->nextElementSibling()) {
        if (child->hasTagName(paramTag)
                && equalIgnoringCase(child->getAttribute(nameAttr), "type")
                && MIMETypeRegistry::isJavaAppletMIMEType(child->getAttribute(valueAttr).string()))
            return true;
        if (child->hasTagName(objectTag)
                && static_cast<HTMLObjectElement*>(child)->containsJavaApplet())
            return true;
        if (child->hasTagName(appletTag))
            return true;
    }

    return false;
}

void HTMLObjectElement::addSubresourceAttributeURLs(ListHashSet<KURL>& urls) const
{
    HTMLPlugInImageElement::addSubresourceAttributeURLs(urls);

    addSubresourceURL(urls, document()->completeURL(getAttribute(dataAttr)));

    // FIXME: Passing a string that starts with "#" to the completeURL function does
    // not seem like it would work. The image element has similar but not identical code.
    const AtomicString& useMap = getAttribute(usemapAttr);
    if (useMap.startsWith("#"))
        addSubresourceURL(urls, document()->completeURL(useMap));
}

void HTMLObjectElement::willMoveToNewOwnerDocument()
{
    FormAssociatedElement::willMoveToNewOwnerDocument();
    HTMLPlugInImageElement::willMoveToNewOwnerDocument();
}

void HTMLObjectElement::insertedIntoTree(bool deep)
{
    FormAssociatedElement::insertedIntoTree();
    HTMLPlugInImageElement::insertedIntoTree(deep);
}

void HTMLObjectElement::removedFromTree(bool deep)
{
    FormAssociatedElement::removedFromTree();
    HTMLPlugInImageElement::removedFromTree(deep);
}

bool HTMLObjectElement::appendFormData(FormDataList& encoding, bool)
{
    if (name().isEmpty())
        return false;

    Widget* widget = pluginWidget();
    if (!widget || !widget->isPluginViewBase())
        return false;
    String value;
    if (!static_cast<PluginViewBase*>(widget)->getFormValue(value))
        return false;
    encoding.appendData(name(), value);
    return true;
}

const AtomicString& HTMLObjectElement::formControlName() const
{
    return m_name.isNull() ? emptyAtom : m_name;
}

HTMLFormElement* HTMLObjectElement::virtualForm() const
{
    return FormAssociatedElement::form();
}

}
