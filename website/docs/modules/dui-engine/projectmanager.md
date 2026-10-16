---
sidebar_position: 4
title: ProjectManager (removed scaffolding)
description: The unused parallel project model was removed in favor of MovieProject.
---

# ProjectManager (removed scaffolding)

The standalone `src/Project` / `ProjectManager` static library was removed in
issue #21. It had no consumers, its locale-dependent wide-stream serializer could
write empty files, and it duplicated the timeline `Clip` definition. It was never
a working substitute for the engine's `.wlmp` model.

Use [MovieMakerCore serialization](../../architecture/moviemakercore/serialization.md)
(`StoryboardManager::MovieProject`) and the in-core SundanceApp `ProjectManager`
for application project I/O. The latter is a different class and remains intact.
This page preserves incoming documentation links; there is no longer a standalone
CMake or CI target named `ProjectManager`.
