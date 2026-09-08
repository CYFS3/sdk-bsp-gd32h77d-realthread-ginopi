"""Gino settings appended to the pinned documentation template's conf.py."""

# This site publishes the current checkout as HTML, with no generated PDF.
html_js_files = [
    "language_switch.js",
    "navigation_state.js",
    "page_outline.js",
]
html_css_files = ["custom.css", "language_switch.css", "dark_mode.css"]
html_context.pop("edit_base_url", None)
myst_heading_anchors = 4
