// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOW_MANAGER_STACKING_MANAGER_H_
#define WINDOW_MANAGER_STACKING_MANAGER_H_

#include <map>
#include <set>
#include <tr1/memory>

#include <gtest/gtest_prod.h>  // for FRIEND_TEST() macro

#include "window_manager/clutter_interface.h"
#include "window_manager/x_types.h"

namespace window_manager {

class Window;
class XConnection;

// Used to stack X11 client windows and Clutter actors.  StackingManager creates
// a window and an actor to use as reference points for each logical stacking
// layer and provides methods to move windows and actors between layers.
class StackingManager {
 public:
  // The layer reference points will be created at the top of the current stack
  // of X windows and children of the default Clutter stage.
  StackingManager(XConnection* xconn, ClutterInterface* clutter);
  ~StackingManager();

  // Layers into which windows can be stacked, in top-to-bottom order.
  enum Layer {
    // Debugging objects that should be positioned above everything else.
    LAYER_DEBUGGING = 0,

    // Hotkey overlay images.
    LAYER_HOTKEY_OVERLAY,

    // A panel as it's being dragged.  This is a separate layer so that the
    // panel's shadow will be cast over stationary panels.
    LAYER_DRAGGED_PANEL,

    // A transient window belonging to the currently-active toplevel
    // window.  Transients are stacked here when in active mode so that
    // they'll obscure panels.  (In overview mode, they're stacked directly
    // above their owners.)
    LAYER_ACTIVE_TRANSIENT_WINDOW,

    // Panel bar's input windows.
    LAYER_PANEL_BAR_INPUT_WINDOW,

    // A stationary panel in the panel bar.
    LAYER_STATIONARY_PANEL_IN_BAR,

    // A stationary panel in a panel dock.
    LAYER_STATIONARY_PANEL_IN_DOCK,

    // Panel docks along the sides of the screen (specifically, their
    // backgrounds).
    LAYER_PANEL_DOCK,

    // Window representing a Chrome tab as it's being dragged out of the
    // tab summary window.
    LAYER_FLOATING_TAB,

    // Tab summary popup displayed when hovering over a window in overview
    // mode.
    LAYER_TAB_SUMMARY,

    // Toplevel windows, along with their transient windows and input
    // windows.
    LAYER_TOPLEVEL_WINDOW,

    // The background image.
    LAYER_BACKGROUND,

    kNumLayers,
  };

  // Is the passed-in X window one of our internal windows?
  bool IsInternalWindow(XWindow xid) {
    return (xids_.find(xid) != xids_.end());
  }

  // Stack a window (both its X window and its Clutter actor) at the top of the
  // passed-in layer.  Its shadow will be stacked at the bottom of the layer so
  // as to not appear above the windows' siblings.  Returns false if the X
  // request fails.
  bool StackWindowAtTopOfLayer(Window* win, Layer layer);

  // Stack an X window at the top of the passed-in layer.  This is useful for X
  // windows that don't have Window objects associated with them (e.g. input
  // windows).  Returns false if the X request fails.
  bool StackXidAtTopOfLayer(XWindow xid, Layer layer);

  // Stack a Clutter actor at the top of the passed-in layer.
  void StackActorAtTopOfLayer(ClutterInterface::Actor* actor, Layer layer);

  // Stack a window's client and composited windows directly above or below
  // another window.  As in StackWindowAtTopOfLayer(), the window's shadow
  // will be stacked at the bottom of 'layer'.  Make sure that 'sibling' is
  // in 'layer' -- things will get confusing otherwise.
  bool StackWindowRelativeToOtherWindow(
      Window* win, Window* sibling, bool above, Layer layer);

 private:
  FRIEND_TEST(LayoutManagerTest, InitialWindowStacking);  // uses 'layer_to_*'

  // Get a layer's name.
  static const char* LayerToName(Layer layer);

  // Get the actor or XID for a particular layer.  These crash if the layer
  // is invalid.
  ClutterInterface::Actor* GetActorForLayer(Layer layer);
  XWindow GetXidForLayer(Layer layer);

  XConnection* xconn_;  // not owned

  // Maps from layers to the corresponding X or Clutter reference points.
  // The reference points are stacked at the top of their corresponding
  // layer (in other words, the Stack*AtTopOfLayer() methods will stack
  // windows and actors directly beneath the corresponding reference
  // points).
  std::map<Layer, XWindow> layer_to_xid_;
  std::map<Layer, std::tr1::shared_ptr<ClutterInterface::Actor> >
      layer_to_actor_;

  // Set we can use for quick lookup of whether an X window belongs to us.
  std::set<XWindow> xids_;
};

}  // namespace window_manager

#endif  // WINDOW_MANAGER_STACKING_MANAGER_H_
