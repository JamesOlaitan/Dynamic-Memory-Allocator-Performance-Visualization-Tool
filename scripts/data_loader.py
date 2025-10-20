import warnings
from typing import Optional

import pandas as pd


class DataLoader:
    """
    A class for loading and preprocessing performance data from CSV files.

    Attributes
    ----------
    file_path : str
        The path to the CSV file containing performance data.

    Methods
    -------
    load_data() -> pd.DataFrame
        Loads data from the CSV file into a pandas DataFrame.
    preprocess_data(df: pd.DataFrame) -> pd.DataFrame
        Cleans and preprocesses the DataFrame.
    """

    def __init__(self, file_path: str):
        """
        Initializes the DataLoader with the specified file path.

        Parameters
        ----------
        file_path : str
            The path to the CSV file containing performance data.
        """
        self.file_path = file_path

    def load_data(self) -> Optional[pd.DataFrame]:
        """
        Loads data from the CSV file into a pandas DataFrame.

        Returns
        -------
        Optional[pd.DataFrame]
            The loaded DataFrame, or None if an error occurred.
        """
        try:
            df = pd.read_csv(self.file_path)
            print(f"Data loaded successfully from {self.file_path}")
            return df
        except FileNotFoundError:
            print(f"Error: File not found at {self.file_path}")
        except pd.errors.EmptyDataError:
            print(f"Error: File at {self.file_path} is empty")
        except pd.errors.ParserError:
            print(f"Error: File at {self.file_path} could not be parsed")
        except Exception as e:
            print(f"An unexpected error occurred: {e}")
        return None

    def preprocess_data(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        Cleans and preprocesses the DataFrame.

        Parameters
        ----------
        df : pd.DataFrame
            The DataFrame to preprocess.

        Returns
        -------
        pd.DataFrame
            The preprocessed DataFrame.
        """
        # Converts 'Timestamp' to datetime using known formats and suppressing parser warnings
        if 'Timestamp' in df.columns:
            df['Timestamp'] = self._parse_timestamps(df['Timestamp'])

        # Converts 'Operation' to categorical
        df['Operation'] = df['Operation'].astype('category')

        # Ensures numerical columns are of correct data type
        df['BlockSize'] = pd.to_numeric(df['BlockSize'], errors='coerce')
        df['Time'] = pd.to_numeric(df['Time'], errors='coerce')
        df['Fragmentation'] = pd.to_numeric(df['Fragmentation'], errors='coerce')

        # Converts 'MemoryAddress' to string (in case it's read as numeric)
        df['MemoryAddress'] = df['MemoryAddress'].astype(str)

        # Converts 'ThreadID' to categorical
        df['ThreadID'] = df['ThreadID'].astype('category')

        # Converts 'AllocationID' to string
        df['AllocationID'] = df['AllocationID'].astype(str)

        # Handles 'Source' and 'CallStack' columns (ensure they are strings)
        df['Source'] = df['Source'].astype(str)
        df['CallStack'] = df['CallStack'].astype(str)

        # Drops rows with any NaN values that resulted from conversion errors
        df = df.dropna()

        # Resets index after dropping rows
        df = df.reset_index(drop=True)

        print("Data preprocessing completed.")
        return df

    @staticmethod
    def _parse_timestamps(series: pd.Series) -> pd.Series:
        """
        Parse timestamp strings using known formats, falling back silently when necessary.
        """
        parsed = pd.Series(pd.NaT, index=series.index, dtype="datetime64[ns]")
        valid_mask = series.notna()
        formats = ("%Y-%m-%d %H:%M:%S", "%Y-%m-%d %H:%M:%S.%f")

        for fmt in formats:
            remaining = valid_mask & parsed.isna()
            if not remaining.any():
                break
            converted = pd.to_datetime(series[remaining], format=fmt, errors="coerce")
            parsed.loc[remaining] = converted

        remaining = valid_mask & parsed.isna()
        if remaining.any():
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", category=UserWarning)
                fallback = pd.to_datetime(series[remaining], errors="coerce")
            parsed.loc[remaining] = fallback

        return parsed
