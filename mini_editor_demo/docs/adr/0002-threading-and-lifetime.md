# ADR-0002: Threading and lifetime

- Status: Accepted
- Date: 2026-09-21

## Decision

The MFC main thread owns COM initialization, engine object creation, engine
commands, unload, release, and COM uninitialization. Qt/QML reports intent and
receives immutable state updates. Engine callbacks marshal events to the owner
before controller or QML state changes.

Shutdown order is:

1. stop accepting UI commands;
2. detach or cancel callbacks;
3. stop playback and pending seeks;
4. hide/detach the preview using the selected transport's safe path;
5. unload source/timeline state;
6. release engine interfaces in reverse ownership order;
7. uninitialize COM on the same thread;
8. unload QML/QtKit state.

## Evidence

- Verified: PDR documents MFC, Qt, SIMBA command, and render threads as distinct.
- Verified: PDR requires `DisplayPlayer` public methods on the main thread.
- Verified: `CSimbaWrapper` documents that cross-thread
  `SetDisplayWnd(NULL)` teardown can deadlock.
- Verified: `MediaObjWrapper` owns a command queue for asynchronous seek work.
- Inferred: one owner thread is the smallest safe policy for this standalone
  project even where a specific low-level API may tolerate other threads.

## Consequences

- Qt callbacks enqueue commands instead of invoking the engine directly.
- Commands carry generation IDs so stale work from a replaced media item can be
  ignored.
- Callback targets use cancellable lifetime tokens, not raw controller pointers.
- No synchronous Qt-to-main call may perform file loading or graph creation.

## Verification

`VR-THREAD-01`, `VR-LIFE-01`, `VR-TRANSPORT-01`.

