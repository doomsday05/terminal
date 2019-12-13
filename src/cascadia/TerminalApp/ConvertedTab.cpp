#include "pch.h"
#include "ConvertedTab.h"
#include "ConvertedTab.g.cpp"

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Microsoft::Terminal::Settings;
using namespace winrt::Microsoft::Terminal::TerminalControl;

namespace winrt::TerminalApp::implementation
{
    ConvertedTab::ConvertedTab(const GUID& profile, const TermControl& control)
    {
        _rootPane = std::make_shared<Pane>(profile, control, true);
        
        _rootPane->Closed([=](auto&& /*s*/, auto&& /*e*/) {
            _ClosedHandlers(nullptr, nullptr);
        });

        _activePane = _rootPane;
    }

    UIElement ConvertedTab::GetRootElement()
    {
        return _rootPane->GetRootElement();
    }

    // Method Description:
    // - Returns nullptr if no children of this tab were the last control to be
    //   focused, or the TermControl that _was_ the last control to be focused (if
    //   there was one).
    // - This control might not currently be focused, if the tab itself is not
    //   currently focused.
    // Arguments:
    // - <none>
    // Return Value:
    // - nullptr if no children were marked `_lastFocused`, else the TermControl
    //   that was last focused.
    TermControl ConvertedTab::GetActiveTerminalControl() const
    {
        return _activePane->GetTerminalControl();
    }

    // Method Description:
    // - Returns true if this is the currently focused tab. For any set of tabs,
    //   there should only be one tab that is marked as focused, though each tab has
    //   no control over the other tabs in the set.
    // Arguments:
    // - <none>
    // Return Value:
    // - true iff this tab is focused.
    bool ConvertedTab::IsFocused() const noexcept
    {
        return _focused;
    }

    // Method Description:
    // - Updates our focus state. If we're gaining focus, make sure to transfer
    //   focus to the last focused terminal control in our tree of controls.
    // Arguments:
    // - focused: our new focus state. If true, we should be focused. If false, we
    //   should be unfocused.
    // Return Value:
    // - <none>
    void ConvertedTab::SetFocused(const bool focused)
    {
        _focused = focused;

        if (_focused)
        {
            _Focus();
        }
    }

    // Method Description:
    // - Returns nullopt if no children of this tab were the last control to be
    //   focused, or the GUID of the profile of the last control to be focused (if
    //   there was one).
    // Arguments:
    // - <none>
    // Return Value:
    // - nullopt if no children of this tab were the last control to be
    //   focused, else the GUID of the profile of the last control to be focused
    std::optional<GUID> ConvertedTab::GetFocusedProfile() const noexcept
    {
        return _activePane->GetFocusedProfile();
    }

    // Method Description:
    // - Focus the last focused control in our tree of panes.
    // Arguments:
    // - <none>
    // Return Value:
    // - <none>
    void ConvertedTab::_Focus()
    {
        _focused = true;

        auto lastFocusedControl = GetActiveTerminalControl();
        if (lastFocusedControl)
        {
            lastFocusedControl.Focus(FocusState::Programmatic);
        }
    }

    // Method Description:
    // - Attempts to update the settings of this tab's tree of panes.
    // Arguments:
    // - settings: The new TerminalSettings to apply to any matching controls
    // - profile: The GUID of the profile these settings should apply to.
    // Return Value:
    // - <none>
    void ConvertedTab::UpdateSettings(const TerminalSettings& settings, const GUID& profile)
    {
        _rootPane->UpdateSettings(settings, profile);
    }

    // Method Description:
    // - Move the viewport of the terminal up or down a number of lines. Negative
    //      values of `delta` will move the view up, and positive values will move
    //      the viewport down.
    // Arguments:
    // - delta: a number of lines to move the viewport relative to the current viewport.
    // Return Value:
    // - <none>
    void ConvertedTab::Scroll(const int delta)
    {
        auto control = GetActiveTerminalControl();
        control.Dispatcher().RunAsync(CoreDispatcherPriority::Normal, [control, delta]() {
            const auto currentOffset = control.GetScrollOffset();
            control.KeyboardScrollViewport(currentOffset + delta);
        });
    }

    void ConvertedTab::UpdateIcon(const winrt::hstring iconPath)
    {
        // Don't reload our icon if it hasn't changed.
        if (iconPath == IconPath())
        {
            return;
        }

        IconPath(iconPath);
    }

    // Method Description:
    // - Gets the title string of the last focused terminal control in our tree.
    //   Returns the empty string if there is no such control.
    // Arguments:
    // - <none>
    // Return Value:
    // - the title string of the last focused terminal control in our tree.
    winrt::hstring ConvertedTab::GetActiveTitle() const
    {
        const auto lastFocusedControl = GetActiveTerminalControl();
        return lastFocusedControl ? lastFocusedControl.Title() : L"";
    }

    // Method Description:
    // - Determines whether the focused pane has sufficient space to be split.
    // Arguments:
    // - splitType: The type of split we want to create.
    // Return Value:
    // - True if the focused pane can be split. False otherwise.
    bool ConvertedTab::CanSplitPane(winrt::TerminalApp::SplitState splitType)
    {
        return _activePane->CanSplit(splitType);
    }

    // Method Description:
    // - Update the size of our panes to fill the new given size. This happens when
    //   the window is resized.
    // Arguments:
    // - newSize: the amount of space that the panes have to fill now.
    // Return Value:
    // - <none>
    void ConvertedTab::ResizeContent(const winrt::Windows::Foundation::Size& newSize)
    {
        // NOTE: This _must_ be called on the root pane, so that it can propogate
        // throughout the entire tree.
        _rootPane->ResizeContent(newSize);
    }

    // Method Description:
    // - Attempt to move a separator between panes, as to resize each child on
    //   either size of the separator. See Pane::ResizePane for details.
    // Arguments:
    // - direction: The direction to move the separator in.
    // Return Value:
    // - <none>
    void ConvertedTab::ResizePane(const winrt::TerminalApp::Direction& direction)
    {
        // NOTE: This _must_ be called on the root pane, so that it can propogate
        // throughout the entire tree.
        _rootPane->ResizePane(direction);
    }

    // Method Description:
    // - Attempt to move focus between panes, as to focus the child on
    //   the other side of the separator. See Pane::NavigateFocus for details.
    // Arguments:
    // - direction: The direction to move the focus in.
    // Return Value:
    // - <none>
    void ConvertedTab::NavigateFocus(const winrt::TerminalApp::Direction& direction)
    {
        // NOTE: This _must_ be called on the root pane, so that it can propogate
        // throughout the entire tree.
        _rootPane->NavigateFocus(direction);
    }

    // Method Description:
    // - Closes the currently focused pane in this tab. If it's the last pane in
    //   this tab, our Closed event will be fired (at a later time) for anyone
    //   registered as a handler of our close event.
    // Arguments:
    // - <none>
    // Return Value:
    // - <none>
    void ConvertedTab::ClosePane()
    {
        _activePane->Close();
    }

    // Method Description:
    // - Register any event handlers that we may need with the given TermControl.
    //   This should be called on each and every TermControl that we add to the tree
    //   of Panes in this tab. We'll add events too:
    //   * notify us when the control's title changed, so we can update our own
    //     title (if necessary)
    // Arguments:
    // - control: the TermControl to add events to.
    // Return Value:
    // - <none>
    void ConvertedTab::_AttachEventHandlersToControl(const TermControl& control)
    {
        auto weakThis{ get_weak() };

        control.TitleChanged([weakThis](auto newTitle) {
            // Check if Tab's lifetime has expired
            if (auto tab{ weakThis.get() })
            {
                tab->Title(tab->GetActiveTitle());
            }
        });
    }

    // Method Description:
    // - Add an event handler to this pane's GotFocus event. When that pane gains
    //   focus, we'll mark it as the new active pane. We'll also query the title of
    //   that pane when it's focused to set our own text, and finally, we'll trigger
    //   our own ActivePaneChanged event.
    // Arguments:
    // - <none>
    // Return Value:
    // - <none>
    void ConvertedTab::_AttachEventHandlersToPane(std::shared_ptr<Pane> pane)
    {
        auto weakThis{ get_weak() };

        pane->GotFocus([weakThis](std::shared_ptr<Pane> sender) {
            // Do nothing if the Tab's lifetime is expired or pane isn't new.
            auto tab{ weakThis.get() };
            if (tab && sender != tab->_activePane)
            {
                // Clear the active state of the entire tree, and mark only the sender as active.
                tab->_rootPane->ClearActive();
                tab->_activePane = sender;
                tab->_activePane->SetActive();

                // Update our own title text to match the newly-active pane.
                tab->Title(tab->GetActiveTitle());

                // Raise our own ActivePaneChanged event.
                tab->_ActivePaneChangedHandlers();
            }
        });
    }

    DEFINE_EVENT(ConvertedTab, PropertyChanged, _PropertyChanged, Windows::UI::Xaml::Data::PropertyChangedEventHandler);
    DEFINE_EVENT(ConvertedTab, ActivePaneChanged, _ActivePaneChangedHandlers, winrt::delegate<>);
}