import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from typing import Optional


class Visualizer:
    """
    A class for generating visualizations from performance data.

    Methods
    -------
    line_plot_time(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Generates a line plot of allocation/deallocation times over operations.
    bar_chart_block_size(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Generates a bar chart comparing average times for different block sizes.
    heatmap_fragmentation(df: pd.DataFrame, output_path: Optional[str] = None) -> None
        Generates a heatmap of fragmentation over time.
    """

    def __init__(self):
        """
        Initializes the Visualizer class.
        """
        sns.set_theme(style="whitegrid")

    def line_plot_time(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Generates a line plot of allocation/deallocation times over operations.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None

        Raises
        ------
        Exception
            If an error occurs while saving or displaying the plot.
        """
        try:
            plt.figure(figsize=(12, 6))
            sns.lineplot(
                data=df,
                x='Timestamp',
                y='Time',
                hue='Operation',
                marker='o'
            )
            plt.title('Allocation/Deallocation Times Over Time')
            plt.xlabel('Timestamp')
            plt.ylabel('Time (seconds)')
            plt.legend(title='Operation')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Line plot saved to {output_path}")
                plt.close()
            else:
                plt.show()
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the line plot: {e}")

    def bar_chart_block_size(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Generates a bar chart comparing average times for different block sizes.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None

        Raises
        ------
        Exception
            If an error occurs while saving or displaying the plot.
        """
        try:
            plt.figure(figsize=(10, 6))
            avg_times = df.groupby('BlockSize')['Time'].mean().reset_index()
            sns.barplot(
                data=avg_times,
                x='BlockSize',
                y='Time',
                hue='BlockSize',
                palette='viridis',
                dodge=False,
                legend=False
            )
            plt.title('Average Allocation Time by Block Size')
            plt.xlabel('Block Size (bytes)')
            plt.ylabel('Average Time (seconds)')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Bar chart saved to {output_path}")
                plt.close()
            else:
                plt.show()
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the bar chart: {e}")

    def heatmap_fragmentation(self, df: pd.DataFrame, output_path: Optional[str] = None) -> None:
        """
        Generates a heatmap of fragmentation over time.

        Parameters
        ----------
        df : pd.DataFrame
            The preprocessed DataFrame containing performance data.
        output_path : Optional[str], default=None
            The file path to save the plot image. If None, the plot is displayed.

        Returns
        -------
        None

        Raises
        ------
        Exception
            If an error occurs while saving or displaying the plot.
        """
        try:
            # Prepare data for heatmap
            df_heatmap = df.pivot_table(
                index=df['Timestamp'].dt.strftime('%Y-%m-%d %H:%M:%S'),
                columns='Operation',
                values='Fragmentation',
                aggfunc='mean'
            ).fillna(0)

            plt.figure(figsize=(12, 8))
            sns.heatmap(
                df_heatmap,
                annot=False,
                fmt=".2f",
                cmap='coolwarm',
                cbar_kws={'label': 'Fragmentation'}
            )
            plt.title('Fragmentation Heatmap Over Time')
            plt.xlabel('Operation')
            plt.ylabel('Timestamp')
            plt.tight_layout()

            if output_path:
                plt.savefig(output_path)
                print(f"Heatmap saved to {output_path}")
                plt.close()
            else:
                plt.show()
                plt.close()
        except Exception as e:
            print(f"An error occurred while generating the heatmap: {e}")