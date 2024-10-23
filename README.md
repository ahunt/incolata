# FTL ("Fit Test Logger")

This is a very hacky proof of concept, and entirely unvalidated, fit-testing
application. It's expected to work with any PortaCount 8020 and 8020A, however
it's only been tested with a single 8020A. This codebase is extremely messy.

![Main Window, showing a test in progress](/docs/img/screenshot_main_2024_10_22.png?raw=true "Screenshot of the test window")

## Setup guide

TODO: write this.

## Architecture

Actual test machinery will live in libp8020a. FTL is just a (not so) nice (yet)
GUI wrapper around that; in due time FTL will (probably) take care of test
logging.

There is no real architecture to speak of, yet.
