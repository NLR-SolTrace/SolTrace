#include "view_module.h"

namespace SolTrace::GUI::App {

ViewModule::ViewModule(QObject* parent)
    : QObject { parent },
      m_left_panel(new SplitPanelData(this)),
      m_right_panel(new SplitPanelData(this)),
      m_sim(new SimulationViewState(this)) {
    connect(this, &ViewModule::workflow_phase_changed, this, [this] {
        set_simulation_content_view(m_workflow_phase == WorkflowPhase::Analyze);
    });
}

void SplitPanelData::show() {
    set_visible(true);
}

void SplitPanelData::hide() {
    set_visible(false);
}

SplitPanelData::SplitPanelData(QObject* parent) : QObject(parent) { }

void ViewModule::fit_panels(int  available_width,
                            bool expanding_right_panel,
                            bool resizing_window,
                            int  margin) {
    QPointer<SplitPanelData> expanding =
        expanding_right_panel ? m_right_panel : m_left_panel;
    QPointer<SplitPanelData> collapsing =
        expanding_right_panel ? m_left_panel : m_right_panel;

    constexpr int left_minimum  = 400;
    constexpr int right_minimum = 250;
    const int     expanding_minimum =
        expanding_right_panel ? right_minimum : left_minimum;
    const int collapsing_minimum =
        expanding_right_panel ? left_minimum : right_minimum;

    int overflow = (m_left_panel->visible() ? m_left_panel->width() : 0) +
                   (m_right_panel->visible() ? m_right_panel->width() : 0) +
                   margin - available_width;
    if (overflow <= 0) return;

    if (collapsing->visible()) {
        const int shrink =
            qMin(overflow, collapsing->width() - collapsing_minimum);
        if (shrink > 0) {
            collapsing->set_width(collapsing->width() - shrink);
            overflow -= shrink;
        }
    }

    if (overflow > 0 && collapsing->visible() && !resizing_window) {
        collapsing->hide();
        return;
    }

    if (overflow > 0 && resizing_window) {
        const int shrink =
            qMin(overflow, expanding->width() - expanding_minimum);
        if (shrink > 0) expanding->set_width(expanding->width() - shrink);
    }
}

void ViewModule::open_left_panel(int available_width) {
    m_left_panel->show();
    fit_panels(available_width);
}

void ViewModule::close_left_panel() {
    m_left_panel->hide();
}

void ViewModule::toggle_left_panel(int available_width) {
    if (m_left_panel->visible()) {
        close_left_panel();
        return;
    }
    open_left_panel(available_width);
}

void ViewModule::open_right_panel(int available_width) {
    m_right_panel->show();
    fit_panels(available_width, true);
}

void ViewModule::close_right_panel() {
    m_right_panel->hide();
}

void ViewModule::toggle_right_panel(int available_width) {
    if (m_right_panel->visible()) {
        close_right_panel();
        return;
    }
    open_right_panel(available_width);
}


} // namespace SolTrace::GUI::App
