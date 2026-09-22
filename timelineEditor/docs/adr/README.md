# Architecture decisions - Homework-3

Each ADR records one decision, the evidence behind it, and what it costs.
Evidence carries one of three labels:

- `Verified`: demonstrated by repository code, a passing build, or a recorded
  runtime check.
- `Inferred`: supported by nearby PDR usage but not yet exercised by this
  homework.
- `Unknown`: requires a proof, an owner answer, or a licensing check.

An ADR may be superseded, but is not edited into silence: the replacement names
what it replaces and why.

| ADR | Decision | Status |
|---|---|---|
| [0001](0001-host-inside-powerdirector.md) | Host inside PowerDirector as a command-line mode | Accepted |
| [0002](0002-drive-itimeline-not-simbawrapper.md) | Drive `ITimeline`, never `CSimbaWrapper` | Accepted |
| [0003](0003-qml-timeline-surface.md) | Timeline UI uses the product's Qt module pattern | Accepted, revised |
| [0004](0004-snapshot-render-command-model.md) | Snapshot rendering, command intent, revert-point undo | Accepted |

## Open questions

Carried from the `Unknown` labels above; each is owned by a ticket.

| Question | ADR | Owner ticket |
|---|---|---|
| Does a `CQtModelessController` present correctly from a command-line host with no main panel behind it? | 0003 | H3-01 (still open; H3-01 shipped the window and pump, not a QML surface) |
| Do the four editing commands map onto `ITimeline` without extending it? | 0002 | H3-05 |

## Answered

**Does an interactive session need different engine settings than the headless
test path?** (ADR-0001/0002, answered in H3-02.) Yes. The product splits the
two postures cleanly:

| Posture | Callers | Setting |
|---|---|---|
| Preview-only | `CMcpHostWindow`, `EngineTestHostWindow`, `CICDProcedure` | `SetSuppressEditMode(true)` |
| Interactive editing | launcher, main panel, `CBatchHostWindow`, `StatusCenter` | `SetSuppressEditMode(false)` + `RestoreEditMode()` |

Homework-3 takes the second, but the reason recorded here at first was wrong
and is corrected as of H3-03. **Edit mode does not gate whether Simba accepts a
timeline write.** Every caller of `IsEnableEditMode()` is in display/preview
code (`displaypanel.cpp`, frame-wait logic) -- none of it is a timeline
mutation path. `McpToolDispatcher` proves it directly: MCP performs real
`HeadlessTimeline` edits (`add_clip` and the rest of the Iris tool set) with
edit mode **suppressed**. Copying `EngineTestHostWindow` verbatim would not
have dropped edits; that specific claim was wrong.

What edit mode actually controls is the **display panel's live-scrub preview
behavior** during an edit -- whether the shown frame updates the way an
interactive editor's does, versus a plainer preview path. Homework-3 still
takes the interactive posture, because H3-06's preview wants that live-edit
frame behavior (matching what `CBatchHostWindow` sets up for its own
produce-verification renders), not because the alternative would silently
no-op writes.

**How does a QML preview receive frames?** (ADR-0003, asked of H3-06, answered
early while researching the above.) Through `EnterRenderlessMode(callback)`.
The launcher and batch pass `nullptr` for a genuinely headless session, but
`AutoEditPreviewController` and `AIEffectMaskController` pass a Qt view
controller and receive `ID3D11Texture2D` frames. So the preview route is an
argument change at an existing call site, not a different engine mode. H3-02
passes `nullptr`; H3-06 replaces it.

This narrows but does not close the remaining H3-06 question, which was really
about window composition rather than frame delivery:

| Question | ADR | Owner ticket |
|---|---|---|
| Do the preview surface and the QML window cooperate in one frame? | 0003 | H3-06 |
