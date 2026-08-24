# Security Policy

## Supported versions

Security fixes are provided for the latest released version. The `main` branch
may contain fixes before the next release, but it is not a supported production
distribution.

## Reporting a vulnerability

Please use GitHub's private vulnerability reporting for this repository. Do not
file a public issue for a suspected vulnerability or publish a proof of
concept before the maintainers have had a chance to investigate it. If private
reporting is unavailable, contact the maintainers through the private channel
listed in the repository hosting settings.

Reports should include the affected version or commit, host runtime, operating
system, input format, a minimal reproduction when it can be shared safely, and
the impact you observed. For a potentially malicious CAD file, provide a
sanitized reproducer or a hash and describe the smallest operation that
triggers the problem.

This policy covers the TypeScript clients, the worker boundary, the WebAssembly
artifacts, parsers and exporters, build scripts, and release supply-chain
inputs. Resource exhaustion and crashes caused by untrusted CAD data are in
scope even when they do not provide code execution.

## Public issue guidance

Use the public issue tracker for ordinary bugs and feature requests. Remove
credentials, proprietary CAD data, and other sensitive material before posting.
