package com.explodingpixels.widgets;

import javax.swing.Action;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import java.util.Arrays;
import java.util.List;

public class PopupMenuCustomizerUsingActions {

    private final List<Action> fActions;

    public PopupMenuCustomizerUsingActions(Action... actions) {
        this(Arrays.asList(actions));
    }

    public PopupMenuCustomizerUsingActions(List<Action> actions) {
        fActions = actions;
    }

    public void customizePopup(JPopupMenu popup) {
        popup.removeAll();
        for (Action action : fActions) {
            JMenuItem menuItem = new JMenuItem(action);
            popup.add(menuItem);
        }

    }

}
