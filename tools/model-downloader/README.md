# Kipepeo Model Downloader

Automated tool to download GGUF quantized language models from HuggingFace for offline AI inference in Kipepeo.

## Features

- ✅ Download models from HuggingFace Hub
- ✅ Resume interrupted downloads
- ✅ Progress tracking with `tqdm`
- ✅ Dry-run mode for testing
- ✅ Configurable via JSON
- ✅ Automatic placement in Android assets

## Installation

```bash
cd tools/model-downloader
pip install -r requirements.txt
```

## Usage

### List Available Models

```bash
python download_models.py --list
```

### Download Specific Models

```bash
# Download 7B and 34B models
python download_models.py --models 7B,34B

# Download 34B model only (recommended for v1.0.0)
python download_models.py --models 34B
```

### Download All Models

```bash
python download_models.py --all
```

### Dry Run (Preview Only)

```bash
python download_models.py --models 7B --dry-run
```

### Custom Output Directory

```bash
python download_models.py --models 34B --output /path/to/custom/dir
```

## Default Output Location

Models are downloaded to:
```
android/app/src/main/assets/models/
```

This directory is included in the Android APK as app assets for offline access.

## Configuration

Edit `models_config.json` to:
- Add new models
- Update repository IDs
- Change filenames
- Specify file sizes

### Example Configuration

```json
{
  "models": {
    "34B": {
      "repo_id": "kipepeo-ai/africa-llm-34b-gguf",
      "filename": "kipepeo-34b-q4_k_m.gguf",
      "size_mb": 19000,
      "description": "Kipepeo 34B Africa-tuned model"
    }
  }
}
```

## Model Recommendations

| Model | Size | RAM Required | Use Case |
|-------|------|--------------|----------|
| **7B** | ~4.2 GB | 6 GB | Testing, low-end devices |
| **13B** | ~7.6 GB | 10 GB | Regular phones |
| **34B** | ~19 GB | 22 GB | **Target for v1.0.0** |
| **70B** | ~38 GB | 42 GB | High-end devices only |

## Africa-Tuned Models

**Note:** The current configuration uses placeholder models from TheBloke's GGUF collection. For production, replace with official Kipepeo Africa-tuned models:

```
kipepeo-ai/africa-llm-7b-gguf
kipepeo-ai/africa-llm-13b-gguf
kipepeo-ai/africa-llm-34b-gguf
kipepeo-ai/africa-llm-70b-gguf
```

These models will be optimized for:
- Swahili, English, and Sheng languages
- Kenyan cultural context
- Mobile device inference (AfricaQuant)

## Troubleshooting

### Download Fails

- Check internet connection
- Verify HuggingFace repository access
- Check disk space (models are large!)

### Import Errors

```bash
pip install --upgrade huggingface-hub tqdm
```

### Permission Issues

Ensure write permissions for the output directory:
```bash
chmod +w android/app/src/main/assets/models/
```

## Integration with Android Build

Models in `android/app/src/main/assets/models/` are automatically:
1. Packaged into the APK during build
2. Accessible via Android AssetManager at runtime
3. Used by Kipepeo's LLM inference engine

## License

Same as Kipepeo project (Apache 2.0)
