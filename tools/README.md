# FlyKylin Tools

Organized tools for different user roles.

## Directory Structure

```
tools/
├── developer/         # Development environment setup
├── agent/            # Agent-specific tools (future)
└── user/             # End-user tools (future)
```

## Developer Tools

See `developer/README.md` for detailed documentation.

### Quick Reference

```powershell
# Start VS Developer Command Prompt (solves MSVC not found)
.\tools\developer\start-vsdevcmd.ps1

# Verify all dependencies
.\tools\developer\verify-environment.ps1

# Install dependencies
.\tools\developer\install-onnx-runtime.ps1
.\tools\developer\install-protobuf.ps1

# Configure environment (run in VS Dev Prompt)
.\tools\developer\configure-environment.ps1
```

## Tool Management Guidelines

- **developer/**: Scripts for setting up development environment
- **agent/**: Workflow automation scripts (planned)
- **user/**: End-user utility scripts (planned)

All scripts use English to avoid PowerShell encoding issues.

## See Also

- Developer tools: `developer/README.md`
- Getting started: `../docs/GETTING_STARTED.md`
- Sprint status: `../SPRINT0_STATUS.md`
