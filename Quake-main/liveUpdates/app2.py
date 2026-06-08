# app.py
import random
import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go

app = dash.Dash(__name__)

app.layout = html.Div([
    dcc.Graph(id='live-graph'),
    dcc.Interval(
        id='interval-component',
        interval=1000,  # Update every 2 seconds
        n_intervals=0
    )
])


@app.callback(
    Output('live-graph', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph(n):
    x = list(range(10))
    y = [random.randint(0, 100) for _ in range(10)]

    fig = go.Figure(data=[go.Scatter(x=x, y=y, mode='lines+markers')])
    return fig


if __name__ == '__main__':
    app.run_server(debug=True)
