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
| Does an interactive session need different engine settings than the headless test path (`SetSuppressEditMode`)? | 0001, 0002 | H3-02 |
| Does a `CQtModelessController` present correctly from a command-line host with no main panel behind it? | 0003 | H3-01 |
| Do the preview canvas and the QML window cooperate in one frame? | 0003 | H3-06 |
| Do the four editing commands map onto `ITimeline` without extending it? | 0002 | H3-05 |
