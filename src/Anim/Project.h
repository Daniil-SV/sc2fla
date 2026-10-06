#pragma once

#include "Animate/Document/SketchDocument.h"
#include "Animate/Library/LibraryItem/DocumentPage.h"
#include "Animate/Library/LibraryItem/Media/MediaBits.h"

#include <string>
#include <unordered_map>

namespace sc2fla::anim {
    struct Project {
        explicit Project(Animate::Document::SketchDocument& doc) :
            document(doc) {}

        Animate::Document::SketchDocument& document;

        std::unordered_map<std::string, Animate::Library::DocumentPage*> symbols;
        std::unordered_map<std::string, Animate::Library::MediaBits*> media;

        bool HasSymbol(const std::string& key) const { return symbols.find(key) != symbols.end(); }

        Animate::Library::DocumentPage& GetSymbol(const std::string& key) const { return *symbols.at(key); }

        Animate::Library::DocumentPage& MakeSymbol(const std::string& path, Animate::Library::DocumentPage::SymbolType type) {
            auto& page = document.GetController().MakeSymbol(path, type);
            symbols[path] = &page;
            return page;
        }

        Animate::Library::MediaBits& MakeBitmap(const std::string& path) {
            auto& bits = document.GetController().MakeBitmap(path);
            media[path] = &bits;
            return bits;
        }
    };
}
