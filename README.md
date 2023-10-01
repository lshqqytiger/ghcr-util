# GitHub Classroom Utils

## 채점하기

```
root
└week5-2-... (current directory)
    └main
    └ghcr

$ ./ghcr main
```

```
root
└ghcr
└week5-2-... (current directory)
    └main

$ ../ghcr main
```

```
root (current directory)
└ghcr
└week5-2-...
    └main

$ ./ghcr --path ./week5-2-.../ main
```

## Build

```
$ make clean main test
```

## Dependencies

- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## License

MIT License

Copyright (c) 2023 이승훈
