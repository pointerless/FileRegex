# Regular Expression for file matching using JSON pattern files #

## About ##

This header only library is used to match raw files or strings to patterns using JSON files to represent the patterns to match. It can match:

* File extensions
* Line patterns (Exact, Repeated or Unordered)
* Max/min line length number of lines
* Automatic match/no match filenames

and more are coming!

## Reqiurements ##

This library uses [JSON for Modern C++](https://github.com/nlohmann/json/) to parse the pattern files. This needs to be installed or the include path ```<nlohmann/json.hpp>``` must resolve correctly.

## JSON Layout ##

The JSON pattern files must contain the following:

```patterns:[]```, a list containing strings representing each lines pattern.

```pattern_type:""```, a string to specify the type of pattern matching:

* ```"Exact"```     : exactly match the patterns with the lines
* ```"Repeated"```  : repeat the patterns as many times as necessary
* ```"Unordered"``` : match to any of the patterns

The file can contain:

```"extensions":[]```, a list containing the accepted file extensions (```".txt"```, ```".md"``` etc.);

```"line_size_range":[]```, a list containing one or two unsigned integers representing the maximum line length or the minimum and maximum respectively;

```"file_length_range":[]```, a list containing one or two unsigned integers representing the maximum number of lines in the file or the minimum and maximum respectively;

```"always_match":[]```, a list containing strings representing filename patterns to always be matched ignoring other properties;

```"never_match":[]```, a list containing strings representing filename patterns to never be matched ignoring other properties;

```"include_blank:true"```, a boolean to decide whether blank lines should be included in line count and pattern matching.

An example JSON file:

``` JSON
{
    "extensions":["txt"],
    "pattern_type":"Repeated",
    "patterns":["[A-Z][0-9]"],
    "line_size_range":[2],
    "file_length_range":[1, 5],
    "always_match":["*.hpp"],
    "never_match":[".*"],
    "include_blank":false
}
```