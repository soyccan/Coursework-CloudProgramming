# Build

```
CXX=clang++ meson setup build
meson compile -C build
```

# Format

https://mesonbuild.com/Code-formatting.html

```
ninja -C build clang-format
```
