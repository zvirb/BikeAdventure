# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BikeAdventure is a meditative, exploration-focused bike riding game with procedurally generated worlds. Players navigate using only left/right choices at decision points, discovering diverse environments and encounters.

## Architecture

### Core Technologies
- **Game Engine**: Unreal Engine 5.4+ (confirmed via SDLC documentation)
- **Language**: C++ for core systems, Blueprint for rapid prototyping
- **Platforms**: Windows PC, Ubuntu Linux
- **Procedural Generation**: Seed-based world generation with biome transitions
- **Performance Target**: 60+ FPS, <3s loading times, 4GB memory budget

### Key Components
- **World Generation System**: Handles biome transitions, path generation, and discovery placement
- **Decision System**: Binary choice mechanic at intersections
- **Discovery System**: Manages encounters, wildlife, weather events
- **Journal System**: Tracks player discoveries and statistics
- **Render System**: Handles visual presentation and effects

## Development Commands

### Environment Setup
```bash
# Windows setup
./scripts/setup/UE5-Setup-Windows.ps1    # Run as administrator

# Ubuntu setup
./scripts/setup/UE5-Setup-Ubuntu.sh      # Run as normal user
```

### Build and Test
```bash
# Multi-platform build
./scripts/automation/build-all-platforms.sh

# Run automated tests
Engine/Binaries/Linux/UnrealEditor BikeAdventure.uproject -ExecCmds="Automation RunTests BikeAdventure.Unit;Quit" -unattended -nullrhi
```

### Git Operations
```bash
# Git operations
git status                  # Check repository status
git add .                   # Stage changes
git commit -m "message"     # Commit changes
git push origin master      # Push to GitHub

# Submodule management
git submodule update --init --recursive  # Initialize submodules
git submodule update --remote            # Update submodules
```

## Project Structure

```
BikeAdventure/
├── GAME_DESIGN.md         # Complete game design document
├── UnifiedWorkflow/       # AI orchestration submodule
├── docs/                  # Comprehensive SDLC documentation
│   ├── Architecture/      # Technical architecture specs
│   ├── SprintPlans/       # Agile sprint frameworks
│   ├── Standards/         # Coding standards and conventions
│   ├── Testing/           # Test automation frameworks
│   └── Deployment/        # CI/CD and deployment guides
├── scripts/               # Development automation scripts
│   ├── setup/            # Platform setup scripts
│   ├── automation/       # Build and deployment scripts
│   └── ci-cd/           # Continuous integration scripts
├── config/               # Configuration files
│   ├── engine/          # UE5 engine configurations
│   ├── ci-cd/           # Pipeline configurations
│   └── testing/         # Test configurations
├── Source/               # C++ source code (UE5 structure)
│   ├── BikeAdventure/   # Main game module
│   │   ├── Core/        # Core game systems
│   │   ├── Gameplay/    # Gameplay mechanics
│   │   └── Systems/     # Engine systems
│   └── BikeAdventure.Target.cs
├── Content/              # UE5 game assets and blueprints
│   ├── Core/            # Core game assets
│   ├── Gameplay/        # Gameplay-specific assets
│   └── Art/             # Visual and audio assets
├── Config/               # UE5 project configuration
└── Tests/                # Automated test suites
    ├── Unit/            # Unit tests
    ├── Integration/     # Integration tests
    └── Performance/     # Performance tests
```

## Implementation Priorities

1. **Prototype Development**
   - Basic movement and decision system
   - Simple procedural path generation
   - Core game loop

2. **World Generation**
   - Biome system implementation
   - Smooth transitions between biomes
   - Discovery placement algorithms

3. **Visual & Audio**
   - Art style implementation
   - Ambient soundscape
   - Weather and time effects

4. **Polish & Features**
   - Journal system
   - Special modes (Zen, Journey)
   - Performance optimization

## Design Principles

- **Simplicity**: One-button gameplay accessible to all
- **Discovery**: Every path leads somewhere interesting
- **Atmosphere**: Focus on mood and ambiance over complexity
- **No Failure**: Exploration without pressure or wrong choices
- **Procedural Beauty**: Endless unique experiences

## UnifiedWorkflow Integration

This project includes the UnifiedWorkflow submodule for AI-assisted development. When using the workflow:

1. **Phase 0**: Always start with prompt clarification for any task
2. **Research Phase**: Investigate best practices for game development patterns
3. **Implementation**: Use parallel agents for different game systems
4. **Testing**: Validate procedural generation and gameplay mechanics
5. **Documentation**: Keep game design and technical docs synchronized

## AI-Driven Development Framework

This project follows a comprehensive AI-DLC (AI-Driven Life Cycle) framework:

### Three-Phase Development Model
1. **Inception Phase (Sprint 1-2)**: Environment setup, prototyping, architecture design
2. **Construction Phase (Sprint 3-8)**: Feature implementation in 2-week iterations
3. **Operations Phase (Sprint 9+)**: Deployment automation, performance optimization

### Agent Allocation Matrix
- **Implementation Agents (3)**: Core systems, movement/physics, world generation
- **Design Agent (1)**: Game balance, biome design, progression systems  
- **Testing Agent (1)**: Automated testing, performance validation, QA
- **Deployment Agent (1)**: Build automation, release management, CI/CD

### Quality Gates
- **Build Requirements**: Must compile, 80% test coverage, 95% integration test pass rate
- **Performance Targets**: 60 FPS minimum, <4GB memory, <3s loading times

## Key Decisions Made

- **Game Engine**: Unreal Engine 5.4+ (confirmed)
- **Platforms**: Windows PC, Ubuntu Linux (confirmed) 
- **Development Model**: AI-agent driven, 2-week sprints
- **Architecture**: Component-based with C++/Blueprint hybrid
- **Testing**: Automated unit, integration, and performance testing

## Testing Approach

Comprehensive automated testing framework includes:

### Unit Testing
- Bike movement and physics calculations
- Intersection detection algorithms
- Biome generation consistency
- Discovery placement variety

### Integration Testing  
- Complete gameplay flow from start to intersection
- Continuous exploration mechanics
- Save/load system functionality
- Cross-platform compatibility

### Performance Testing
- Frame rate under load (60 FPS target)
- Memory usage monitoring (<4GB limit)
- Loading time validation (<3s requirement)
- Extended play session stability

### Gauntlet Testing
- Automated gameplay scenarios
- User experience validation
- Performance benchmarking
- Platform-specific testing

## Performance Considerations

- Efficient procedural generation without stutters
- Memory management for long sessions
- Asset streaming for seamless exploration
- Battery optimization for mobile platforms