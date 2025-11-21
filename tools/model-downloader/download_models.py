#!/usr/bin/env python3
"""
Kipepeo Model Downloader
Downloads GGUF quantized LLM models from HuggingFace for offline AI inference.

Usage:
    python download_models.py --models 7B,34B
    python download_models.py --all
    python download_models.py --models 7B --dry-run
"""

import argparse
import json
import os
import sys
from pathlib import Path
from typing import Dict, List

try:
    from huggingface_hub import hf_hub_download
    from tqdm import tqdm
except ImportError:
    print("Error: Required dependencies not installed.")
    print("Please run: pip install -r requirements.txt")
    sys.exit(1)


# Configuration
SCRIPT_DIR = Path(__file__).parent
CONFIG_FILE = SCRIPT_DIR / "models_config.json"
DEFAULT_OUTPUT_DIR = SCRIPT_DIR.parent.parent / "android" / "app" / "src" / "main" / "assets" / "models"


def load_config() -> Dict:
    """Load model configuration from JSON file."""
    if not CONFIG_FILE.exists():
        print(f"Error: Configuration file not found: {CONFIG_FILE}")
        sys.exit(1)
    
    with open(CONFIG_FILE, 'r') as f:
        return json.load(f)


def download_model(repo_id: str, filename: str, output_dir: Path, dry_run: bool = False) -> bool:
    """
    Download a single model from HuggingFace.
    
    Args:
        repo_id: HuggingFace repository ID (e.g., 'kipepeo-ai/africa-llm-7b-gguf')
        filename: Name of the file to download
        output_dir: Directory to save the downloaded file
        dry_run: If True, only simulate the download
    
    Returns:
        True if successful, False otherwise
    """
    try:
        output_path = output_dir / filename
        
        if output_path.exists():
            print(f"✓ Model already exists: {filename}")
            return True
        
        if dry_run:
            print(f"[DRY RUN] Would download: {repo_id}/{filename} -> {output_path}")
            return True
        
        print(f"Downloading {filename} from {repo_id}...")
        
        # Create output directory if it doesn't exist
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Download with progress bar
        downloaded_path = hf_hub_download(
            repo_id=repo_id,
            filename=filename,
            local_dir=output_dir,
            local_dir_use_symlinks=False,
            resume_download=True
        )
        
        print(f"✓ Successfully downloaded: {filename}")
        return True
        
    except Exception as e:
        print(f"✗ Failed to download {filename}: {str(e)}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Download Kipepeo AI models from HuggingFace",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Download specific models
  python download_models.py --models 7B,34B
  
  # Download all models
  python download_models.py --all
  
  # Dry run (preview what would be downloaded)
  python download_models.py --models 7B --dry-run
  
  # Custom output directory
  python download_models.py --models 34B --output /path/to/models
        """
    )
    
    parser.add_argument(
        '--models',
        type=str,
        help='Comma-separated list of models to download (e.g., 7B,13B,34B,70B)'
    )
    parser.add_argument(
        '--all',
        action='store_true',
        help='Download all available models'
    )
    parser.add_argument(
        '--output',
        type=str,
        help=f'Output directory (default: {DEFAULT_OUTPUT_DIR})'
    )
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Simulate download without actually downloading files'
    )
    parser.add_argument(
        '--list',
        action='store_true',
        help='List available models and exit'
    )
    
    args = parser.parse_args()
    
    # Load configuration
    config = load_config()
    available_models = config.get('models', {})
    
    # List models if requested
    if args.list:
        print("\nAvailable Models:")
        print("-" * 80)
        for model_name, model_info in available_models.items():
            size_mb = model_info.get('size_mb', 'unknown')
            repo_id = model_info.get('repo_id', 'unknown')
            filename = model_info.get('filename', 'unknown')
            print(f"{model_name:6s} | {size_mb:>8} MB | {repo_id}/{filename}")
        print("-" * 80)
        return 0
    
    # Determine which models to download
    models_to_download: List[str] = []
    
    if args.all:
        models_to_download = list(available_models.keys())
    elif args.models:
        models_to_download = [m.strip() for m in args.models.split(',')]
        # Validate
        invalid_models = [m for m in models_to_download if m not in available_models]
        if invalid_models:
            print(f"Error: Invalid model(s): {', '.join(invalid_models)}")
            print(f"Available models: {', '.join(available_models.keys())}")
            return 1
    else:
        parser.print_help()
        return 1
    
    # Determine output directory
    output_dir = Path(args.output) if args.output else DEFAULT_OUTPUT_DIR
    
    # Print summary
    print("\n" + "="*80)
    print("Kipepeo Model Downloader")
    print("="*80)
    print(f"Models to download: {', '.join(models_to_download)}")
    print(f"Output directory: {output_dir}")
    print(f"Dry run: {args.dry_run}")
    print("="*80 + "\n")
    
    # Download models
    success_count = 0
    fail_count = 0
    
    for model_name in models_to_download:
        model_info = available_models[model_name]
        repo_id = model_info['repo_id']
        filename = model_info['filename']
        
        if download_model(repo_id, filename, output_dir, args.dry_run):
            success_count += 1
        else:
            fail_count += 1
    
    # Print summary
    print("\n" + "="*80)
    print(f"Download Summary: {success_count} succeeded, {fail_count} failed")
    print("="*80)
    
    return 0 if fail_count == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
