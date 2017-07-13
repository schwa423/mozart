// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include "apps/mozart/lib/scene/types.h"
#include "apps/mozart/src/scene/resources/resource_type_info.h"
#include "lib/ftl/memory/ref_counted.h"

namespace mozart {
namespace scene {

class ErrorReporter;
class Session;
class Import;

// Resource is the base class for all client-created objects (i.e. those that
// are created in response to a CreateResourceOp operation).
// TODO: use unsafe ref-counting for better performance (our architecture
// guarantees that this is safe).
class Resource : public ftl::RefCountedThreadSafe<Resource> {
 public:
  static const ResourceTypeInfo kTypeInfo;

  virtual ~Resource();

  const ResourceTypeInfo& type_info() const { return type_info_; }
  ResourceTypeFlags type_flags() const { return type_info_.flags; }
  const char* type_name() const { return type_info_.name; }

  // The session this Resource lives in and the id it was created with there.
  Session* session() const { return session_; }
  ResourceId id() const { return id_; }

  // An error reporter associated with the Resource's session. When operating
  // on this resource, always log errors to the ErrorReporter before failing.
  ErrorReporter* error_reporter() const;

  // The diagnostic label.
  const std::string label() const { return label_; }
  bool SetLabel(const std::string& label);

  // Used by ResourceVisitor to visit a tree of Resources.
  virtual void Accept(class ResourceVisitor* visitor) = 0;

  // Return true if the specified type is identical or a base type of this
  // TypedReffable; return false otherwise.
  // TODO: Move this to a separate class we inherit from.
  template <typename TypedReffableT>
  bool IsKindOf() const {
    return type_info().IsKindOf(TypedReffableT::kTypeInfo);
  }

  // Downcasts the reference to the specified subclass. Throws an exception
  // in debug mode if the type of the object does not match.
  //
  // Example usage: ftl::RefPtr<Subclass> = object.As<Subclass>();
  // TODO: Move this to a separate class we inherit from.
  template <typename T>
  ftl::RefPtr<T> As() {
    FTL_DCHECK(this->IsKindOf<T>());
    return ftl::RefPtr<T>(static_cast<T*>(this));
  }

  /// The list of import resource that currently have a binding to this
  /// resource.
  const std::vector<Import*>& imports() const { return imports_; }

  /// Adds the import resource to the list of importers of this resource.
  virtual void AddImport(Import* import);

  /// Removes the import resource from the list of importers of this resource.
  virtual void RemoveImport(Import* import);

 protected:
  Resource(Session* session, ResourceId id, const ResourceTypeInfo& type_info);

  friend class ResourceMap;
  friend class Import;
  /// For the given resource type info, returns the resource that will act as
  /// the target for ops directed at this resource. Subclasses (notably the
  /// |Import| since their binding are not mutable) may return alternate
  /// resources to act as the recipients of ops.
  virtual Resource* GetDelegate(const ResourceTypeInfo& type_info);

 private:
  Session* const session_;
  const ResourceId id_;
  const ResourceTypeInfo& type_info_;
  std::string label_;
  std::vector<Import*> imports_;
};

using ResourcePtr = ftl::RefPtr<Resource>;

}  // namespace scene
}  // namespace mozart
